#include "power_chart.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include <cstring>
#include <cstdio>
#include <algorithm>

#define TAG "power_chart"

// Forward declaration for task context
typedef struct {
    PowerChart* chart;
    char url[256];
} FetchContext;

// Background task for HTTP fetch
static void FetchDataTask(void* arg) {
    FetchContext* ctx = (FetchContext*)arg;
    PowerChart* chart = ctx->chart;
    const char* url = ctx->url;
    free(ctx);
    
    ESP_LOGI(TAG, "Background fetch: %s", url);
    
    esp_http_client_config_t config = {};
    config.url = url;
    config.timeout_ms = 10000;
    config.event_handler = NULL;
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "HTTP client init failed");
        return;
    }
    
    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP GET failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return;
    }
    
    long content_length = esp_http_client_get_content_length(client);
    if (content_length <= 0 || content_length > 65536) {
        ESP_LOGE(TAG, "Invalid content length: %ld", content_length);
        esp_http_client_cleanup(client);
        return;
    }
    
    char* response = (char*)malloc(content_length + 1);
    if (!response) {
        ESP_LOGE(TAG, "Allocation failed");
        esp_http_client_cleanup(client);
        return;
    }
    
    int total_read = 0;
    int read_bytes;
    while (total_read < content_length) {
        read_bytes = esp_http_client_read(client, response + total_read, content_length - total_read);
        if (read_bytes <= 0) break;
        total_read += read_bytes;
    }
    response[total_read] = '\0';
    esp_http_client_cleanup(client);
    
    // Parse and render back on LVGL task
    if (chart->ParseAndRender(response, total_read)) {
        // Update status on LVGL thread
        lv_obj_t* stat = chart->GetStatusLabel();
        if (stat) {
            lv_label_set_text(stat, "Loaded");
        }
    } else {
        lv_obj_t* stat = chart->GetStatusLabel();
        if (stat) {
            lv_label_set_text(stat, "Parse error");
        }
    }
    
    free(response);
}

PowerChart::PowerChart() {
    ESP_LOGI(TAG, "PowerChart created");
}

PowerChart::~PowerChart() {
    if (refresh_timer_) {
        esp_timer_stop(refresh_timer_);
        esp_timer_delete(refresh_timer_);
        refresh_timer_ = nullptr;
    }
}

void PowerChart::Initialize(lv_obj_t* tabview) {
    tabview_ = tabview;
    ESP_LOGI(TAG, "PowerChart initialized with tabview");
}

void PowerChart::Create(lv_obj_t* power_tab) {
    chart_tab_ = power_tab;
    data_fetched_ = false;
    
    // Clear the tab
    lv_obj_clean(power_tab);
    
    // Full-screen background
    lv_obj_set_size(power_tab, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(power_tab, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(power_tab, 255, LV_PART_MAIN);
    
    // Title label
    title_label_ = lv_label_create(power_tab);
    lv_obj_set_style_text_color(title_label_, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(title_label_, lv_font_default(), LV_PART_MAIN);
    lv_obj_set_style_text_align(title_label_, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_align(title_label_, LV_ALIGN_TOP_LEFT, CHART_X, CHART_Y - 40);
    lv_label_set_text(title_label_, "Power - Last 24 Hours");
    
    // Current value label (stats)
    value_label_ = lv_label_create(power_tab);
    lv_obj_set_style_text_color(value_label_, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(value_label_, lv_font_default(), LV_PART_MAIN);
    lv_obj_set_style_text_align(value_label_, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_align_to(value_label_, title_label_, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
    lv_label_set_text(value_label_, "Loading...");
    
    // Status label
    status_label_ = lv_label_create(power_tab);
    lv_obj_set_style_text_color(status_label_, lv_color_hex(0x808080), LV_PART_MAIN);
    lv_obj_set_style_text_font(status_label_, lv_font_default(), LV_PART_MAIN);
    lv_obj_set_style_text_align(status_label_, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_align_to(status_label_, title_label_, LV_ALIGN_OUT_LEFT_MID, 0, 45);
    lv_label_set_text(status_label_, "Connecting...");
    
    // Draw axes
    DrawAxes();
    
    // Draw empty bars (will be filled with data)
    bar_objects_.clear();
    int x = CHART_X;
    for (int i = 0; i < CHART_POINTS; i++) {
        lv_obj_t* bar = lv_obj_create(power_tab);
        lv_obj_set_size(bar, BAR_WIDTH, 1);
        lv_obj_set_pos(bar, x, CHART_Y + CHART_H - 1);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x404040), LV_PART_MAIN);
        lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN);
        bar_objects_.push_back(bar);
        x += BAR_WIDTH + BAR_GAP;
    }
    
    ESP_LOGI(TAG, "PowerChart UI created with %zu bars", bar_objects_.size());
}

void PowerChart::Show() {
    ESP_LOGI(TAG, "PowerChart shown, starting fetch");
    // Start background fetch task
    std::string ip = GetLocalIP();
    if (ip.empty()) {
        ESP_LOGE(TAG, "No network IP available");
        if (status_label_) {
            lv_label_set_text(status_label_, "No network");
        }
        return;
    }
    
    char url[256];
    snprintf(url, sizeof(url), "http://%s:%d%s%s", ip.c_str(), SHELLY_API_PORT, SHELLY_API_PATH, SHELLY_API_QUERY);
    
    FetchContext* ctx = (FetchContext*)malloc(sizeof(FetchContext));
    ctx->chart = this;
    strncpy(ctx->url, url, sizeof(ctx->url) - 1);
    ctx->url[sizeof(ctx->url) - 1] = '\0';
    
    xTaskCreate(FetchDataTask, "shelly_fetch", 4096, ctx, 5, nullptr);
}

bool PowerChart::ParseAndRender(const char* json_response, int length) {
    cJSON* root = cJSON_ParseWithLength(json_response, length);
    if (!root) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return false;
    }
    
    // Structure: { "data": { "windows": { "hourly": { "totals": [...] } } } }
    cJSON* data = cJSON_GetObjectItem(root, "data");
    if (!data) {
        ESP_LOGW(TAG, "No data object found");
        cJSON_Delete(root);
        return false;
    }
    
    cJSON* windows = cJSON_GetObjectItem(data, "windows");
    if (!windows) {
        ESP_LOGW(TAG, "No windows object found");
        cJSON_Delete(root);
        return false;
    }
    
    cJSON* hourly = cJSON_GetObjectItem(windows, "hourly");
    if (!hourly) {
        ESP_LOGW(TAG, "No hourly window found");
        cJSON_Delete(root);
        return false;
    }
    
    // Get totals array
    cJSON* totals = cJSON_GetObjectItem(hourly, "totals");
    if (totals && cJSON_IsArray(totals)) {
        int count = cJSON_GetArraySize(totals);
        int max_points = std::min(count, CHART_POINTS);
        for (int i = 0; i < max_points; i++) {
            cJSON* item = cJSON_GetArrayItem(totals, i);
            if (item && cJSON_IsNumber(item)) {
                hourly_data_[i] = item->valuedouble;
            }
        }
        data_count_ = max_points;
    }
    
    cJSON_Delete(root);
    
    if (data_count_ == 0) return false;
    
    // Render immediately on LVGL thread
    RenderChart();
    UpdateStats();
    return true;
}

void PowerChart::RenderChart() {
    if (bar_objects_.empty() || data_count_ == 0) return;
    
    // Find max value for scaling
    double max_value = 0;
    for (int i = 0; i < data_count_; i++) {
        max_value = std::max(max_value, hourly_data_[i]);
    }
    if (max_value < 100) max_value = 100;  // Minimum scale
    
    for (int i = 0; i < data_count_ && i < (int)bar_objects_.size(); i++) {
        double bar_height = (hourly_data_[i] / max_value) * CHART_H;
        if (bar_height < 1) bar_height = 1;
        
        lv_obj_t* bar = bar_objects_[i];
        lv_coord_t bar_y = CHART_Y + CHART_H - (lv_coord_t)bar_height;
        
        lv_obj_set_size(bar, BAR_WIDTH, (lv_coord_t)bar_height);
        lv_obj_set_pos(bar, CHART_X + i * (BAR_WIDTH + BAR_GAP), bar_y);
        
        // Color based on power level (dark -> bright)
        int gray = (int)(255 * (hourly_data_[i] / max_value));
        gray = std::min(gray, 255);
        lv_obj_set_style_bg_color(bar, lv_color_make(gray, gray, 0), LV_PART_MAIN);
    }
    
    data_fetched_ = true;
    ESP_LOGI(TAG, "Chart rendered: %d bars, max=%.0fW", data_count_, max_value);
}

void PowerChart::DrawAxes() {
    if (!chart_tab_) return;
    
    // X-axis line
    lv_obj_t* x_axis = lv_line_create(chart_tab_);
    lv_point_t x_points[] = {{CHART_X, CHART_Y + CHART_H}, {CHART_X + CHART_W, CHART_Y + CHART_H}};
    lv_line_set_points(x_axis, x_points, 2);
    lv_obj_set_style_line_color(x_axis, lv_color_hex(0x404040), LV_PART_LINES);
    lv_obj_set_style_line_width(x_axis, 1, LV_PART_LINES);
    
    // Y-axis line
    lv_obj_t* y_axis = lv_line_create(chart_tab_);
    lv_point_t y_points[] = {{CHART_X, CHART_Y}, {CHART_X, CHART_Y + CHART_H}};
    lv_line_set_points(y_axis, y_points, 2);
    lv_obj_set_style_line_color(y_axis, lv_color_hex(0x404040), LV_PART_LINES);
    lv_obj_set_style_line_width(y_axis, 1, LV_PART_LINES);
    
    // Y-axis labels
    const char* y_labels[] = {"0", "50%", "100%"};
    for (int i = 0; i < 3; i++) {
        lv_obj_t* label = lv_label_create(chart_tab_);
        lv_obj_set_style_text_color(label, lv_color_hex(0x606060), LV_PART_MAIN);
        lv_obj_set_style_text_font(label, lv_font_default(), LV_PART_MAIN);
        lv_label_set_text(label, y_labels[i]);
        int y_off = (int)((CHART_H / 2.0) * (1.0 - i * 2.0 / 2.0)) - 5;
        lv_obj_align(label, LV_ALIGN_LEFT_MID, -Y_LABEL_SPACE, CHART_Y + y_off);
    }
    
    // X-axis labels
    const char* x_labels[] = {"24h", "20h", "16h", "12h", "8h", "4h", "Now"};
    for (int i = 0; i < 7; i++) {
        lv_obj_t* label = lv_label_create(chart_tab_);
        lv_obj_set_style_text_color(label, lv_color_hex(0x606060), LV_PART_MAIN);
        lv_obj_set_style_text_font(label, lv_font_default(), LV_PART_MAIN);
        lv_label_set_text(label, x_labels[i]);
        int label_x = CHART_X + (i * CHART_W / 6);
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, label_x - 8, CHART_Y + CHART_H + 15);
    }
}

void PowerChart::UpdateStats() {
    if (!value_label_ || data_count_ == 0) return;
    
    double sum = 0;
    double max = 0;
    for (int i = 0; i < data_count_; i++) {
        sum += hourly_data_[i];
        max = std::max(max, hourly_data_[i]);
    }
    double avg = sum / data_count_;
    
    char buf[80];
    snprintf(buf, sizeof(buf), "Now: %s | Avg: %s | Max: %s",
             FormatPower(hourly_data_[0]).c_str(),
             FormatPower(avg).c_str(),
             FormatPower(max).c_str());
    lv_label_set_text(value_label_, buf);
}

std::string PowerChart::FormatPower(double watts) {
    char buf[16];
    if (watts >= 1000) {
        snprintf(buf, sizeof(buf), "%.1fkW", watts / 1000);
    } else {
        snprintf(buf, sizeof(buf), "%.0fW", watts);
    }
    return std::string(buf);
}

lv_obj_t* PowerChart::GetStatusLabel() const {
    return status_label_;
}

std::string PowerChart::GetLocalIP() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return "";
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(53);
    server_addr.sin_addr.s_addr = inet_addr("8.8.8.8");
    
    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);
    getsockname(sock, (struct sockaddr*)&local_addr, &addr_len);
    
    close(sock);
    
    char ip_str[16];
    inet_ntoa_r(local_addr.sin_addr, ip_str, sizeof(ip_str));
    return std::string(ip_str);
}
