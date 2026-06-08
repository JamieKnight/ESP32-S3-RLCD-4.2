#include "temperature_chart.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <esp_heap_caps.h>

#define TAG "temp_chart"

// Min/max temperature for scaling (ESP32 internal sensor range)
static const float TEMP_MIN = 0.0f;
static const float TEMP_MAX = 60.0f;
static const int BAR_WIDTH = 6;
static const int BAR_COUNT = TEMP_BUFFER_SIZE;
static const int BAR_X_SPACING = 1;

void TemperatureChart::Create(lv_obj_t* parent) {
    ESP_LOGI(TAG, "Creating temperature chart");
    
    // Chart area background
    chart_bg_ = lv_obj_create(parent);
    lv_obj_set_size(chart_bg_, 390, 180);
    lv_obj_set_style_bg_color(chart_bg_, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_border_width(chart_bg_, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(chart_bg_, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_align(chart_bg_, LV_ALIGN_TOP_LEFT, 8, 65);
    
    // Title
    title_label_ = lv_label_create(parent);
    lv_obj_set_style_text_color(title_label_, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(title_label_, lv_font_dejavu_16_persian_ansi, LV_PART_MAIN);
    lv_label_set_text(title_label_, "\u2103 Temperature - Last 24 Hours");
    lv_obj_align(title_label_, LV_ALIGN_TOP_LEFT, 8, 15);
    
    // Waterline (average-ish)
    temp_grid_h_ = lv_line_create(parent);
    static lv_point_t waterline_pts[] = { {8, 6}, {382, 6} };
    lv_line_set_points(temp_grid_h_, waterline_pts, 2);
    lv_obj_set_style_line_color(temp_grid_h_, lv_color_hex(0x555555), LV_PART_MAIN);
    lv_obj_set_style_line_width(temp_grid_h_, 1, LV_PART_MAIN);
    lv_obj_align_to(temp_grid_h_, chart_bg_, LV_ALIGN_TOP_LEFT, 0, (int)(180 * (1 - TEMP_WATERLINE_Y_RATIO)));
    
    // Waterline label
    waterline_label_ = lv_label_create(parent);
    lv_obj_set_style_text_color(waterline_label_, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
    lv_obj_set_style_text_font(waterline_label_, lv_font_dejavu_10_persian_ansi, LV_PART_MAIN);
    lv_label_set_text(waterline_label_, "Avg: ---");
    lv_obj_align_to(waterline_label_, temp_grid_h_, LV_ALIGN_OUT_RIGHT_MID, 2, 0);
    
    // Current temp
    current_label_ = lv_label_create(parent);
    lv_obj_set_style_text_color(current_label_, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(current_label_, lv_font_dejavu_14_persian_ansi, LV_PART_MAIN);
    lv_label_set_text(current_label_, "Current: --.- C");
    lv_obj_align_to(current_label_, chart_bg_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    
    // Stats
    avg_label_ = lv_label_create(parent);
    lv_obj_set_style_text_font(avg_label_, lv_font_dejavu_10_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(avg_label_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_label_set_text(avg_label_, "Avg: --.- C");
    lv_obj_align_to(avg_label_, current_label_, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
    
    max_label_ = lv_label_create(parent);
    lv_obj_set_style_text_font(max_label_, lv_font_dejavu_10_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(max_label_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_label_set_text(max_label_, "Max: --.- C");
    lv_obj_align_to(max_label_, avg_label_, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
    
    min_label_ = lv_label_create(parent);
    lv_obj_set_style_text_font(min_label_, lv_font_dejavu_10_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(min_label_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_label_set_text(min_label_, "Min: --.- C");
    lv_obj_align_to(min_label_, max_label_, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
    
    // Refresh indicator
    refresh_label_ = lv_label_create(parent);
    lv_obj_set_style_text_font(refresh_label_, lv_font_dejavu_10_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(refresh_label_, lv_color_hex(0x666666), LV_PART_MAIN);
    lv_label_set_text(refresh_label_, "Update: ---");
    lv_obj_align(refresh_label_, LV_ALIGN_BOTTOM_LEFT, 8, -5);
    
    // Create bar placeholders
    int total_bar_width = BAR_COUNT * (BAR_WIDTH + BAR_X_SPACING) - BAR_X_SPACING;
    int start_x = (390 - total_bar_width) / 2;
    for (int i = 0; i < BAR_COUNT; i++) {
        bars_[i] = lv_obj_create(parent);
        lv_obj_set_size(bars_[i], BAR_WIDTH, 180);
        lv_obj_set_style_bg_color(bars_[i], lv_color_hex(0x666666), LV_PART_MAIN);
        lv_obj_set_style_radius(bars_[i], 1, LV_PART_MAIN);
        lv_obj_align_to(bars_[i], chart_bg_, LV_ALIGN_TOP_LEFT, start_x + i * (BAR_WIDTH + BAR_X_SPACING), 0);
        lv_obj_add_flag(bars_[i], LV_OBJ_FLAG_HIDDEN);
    }
    
    // Initialize temperature sensor
    temperature_sensor_config_t temp_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);
    ESP_ERROR_CHECK(temperature_sensor_install(&temp_config, &temp_sensor_));
    ESP_ERROR_CHECK(temperature_sensor_enable(temp_sensor_));
    
    ESP_LOGI(TAG, "Temperature sensor initialized");
}

void TemperatureChart::Show() {
    if (!has_data_) {
        StartPolling();
        OnTemperatureSample();  // Initial sample
    }
    RenderChart();
    UpdateStats();
    refresh_label_ && lv_label_set_text(refresh_label_, "Updated now");
}

void TemperatureChart::StartPolling() {
    if (sample_timer_) return;
    
    esp_timer_create_args_t timer_args = {
        .callback = [](void* arg) {
            TemperatureChart* self = static_cast<TemperatureChart*>(arg);
            if (self) {
                self->OnTemperatureSample();
            }
        },
        .arg = this,
        .name = "temp_sample"
    };
    esp_timer_create(&timer_args, &sample_timer_);
    ESP_ERROR_CHECK(esp_timer_start_periodic(sample_timer_, TEMP_SAMPLE_INTERVAL_S * 1000000ULL));
    ESP_LOGI(TAG, "Started temperature polling (%ds interval)", TEMP_SAMPLE_INTERVAL_S);
}

void TemperatureChart::StopPolling() {
    if (sample_timer_) {
        esp_timer_stop(sample_timer_);
        esp_timer_delete(sample_timer_);
        sample_timer_ = nullptr;
    }
}

void TemperatureChart::OnTemperatureSample() {
    float temp = 0.0f;
    if (temp_sensor_ && ESP_OK == temperature_sensor_get_celsius(temp_sensor_, &temp)) {
        // Store in circular buffer
        temp_buffer_[write_index_] = temp;
        write_index_ = (write_index_ + 1) % BAR_COUNT;
        if (buffer_count_ < BAR_COUNT) buffer_count_++;
        has_data_ = true;
        
        char time_buf[32];
        snprintf(time_buf, sizeof(time_buf), "Temp: %.1f C", temp);
        ESP_LOGI(TAG, "%s", time_buf);
    }
}

void TemperatureChart::RenderChart() {
    int count = has_data_ ? buffer_count_ : 0;
    if (count == 0) return;
    
    // Find min/max for scaling
    float min_temp = temp_buffer_[0];
    float max_temp = temp_buffer_[0];
    float sum = 0.0f;
    for (int i = 0; i < count; i++) {
        int idx = (write_index_ - count + i + BAR_COUNT) % BAR_COUNT;
        if (temp_buffer_[idx] < min_temp) min_temp = temp_buffer_[idx];
        if (temp_buffer_[idx] > max_temp) max_temp = temp_buffer_[idx];
        sum += temp_buffer_[idx];
    }
    
    float range = max_temp - min_temp;
    if (range < 1.0f) {
        // Add small range to avoid flat line
        float center = (max_temp + min_temp) / 2.0f;
        min_temp = center - 1.0f;
        max_temp = center + 1.0f;
        range = 2.0f;
    }
    
    int chart_h = 180;
    
    for (int i = 0; i < count; i++) {
        int idx = (write_index_ - count + i + BAR_COUNT) % BAR_COUNT;
        float temp = temp_buffer_[idx];
        
        // Scale: map temp range to chart height
        float norm = (temp - min_temp) / range;
        int bar_h = (int)(norm * chart_h * 0.9);  // 90% of chart height
        int y_offset = chart_h - bar_h;
        
        // Set bar position and height
        lv_obj_set_pos(bars_[i], 0, y_offset);
        lv_obj_set_size(bars_[i], BAR_WIDTH, bar_h);
        
        // Brightness based on temperature
        int brightness = 40 + (int)(215 * norm);
        char hex[16];
        snprintf(hex, sizeof(hex), "0x%02X%02X%02X", brightness, brightness, brightness);
        lv_obj_set_style_bg_color(bars_[i], lv_color_hex(0x666666), LV_PART_MAIN);
        
        lv_obj_remove_flag(bars_[i], LV_OBJ_FLAG_HIDDEN);
    }
    
    // Hide unused bars
    for (int i = count; i < BAR_COUNT; i++) {
        lv_obj_add_flag(bars_[i], LV_OBJ_FLAG_HIDDEN);
    }
    
    // Position bars within chart_bg
    int start_x = 8 + (390 - count * (BAR_WIDTH + BAR_X_SPACING) + BAR_X_SPACING) / 2;
    for (int i = 0; i < count; i++) {
        lv_obj_set_pos(bars_[i], start_x + i * (BAR_WIDTH + BAR_X_SPACING),
                       lv_obj_get_style_y(bars_[i], LV_PART_MAIN));
    }
}

void TemperatureChart::UpdateStats() {
    float sum = 0.0f;
    float min_temp = 999.0f;
    float max_temp = -999.0f;
    
    for (int i = 0; i < buffer_count_; i++) {
        int idx = (write_index_ - buffer_count_ + i + BAR_COUNT) % BAR_COUNT;
        float t = temp_buffer_[idx];
        sum += t;
        if (t < min_temp) min_temp = t;
        if (t > max_temp) max_temp = t;
    }
    
    if (buffer_count_ == 0) return;
    
    float avg = sum / buffer_count_;
    float current = temp_buffer_[(write_index_ - 1 + BAR_COUNT) % BAR_COUNT];
    
    char cur_buf[32];
    snprintf(cur_buf, sizeof(cur_buf), "Current: %.1f C", current);
    lv_label_set_text(current_label_, cur_buf);
    
    char avg_buf[32];
    snprintf(avg_buf, sizeof(avg_buf), "Avg: %.1f C", avg);
    lv_label_set_text(avg_label_, avg_buf);
    
    char max_buf[32];
    snprintf(max_buf, sizeof(max_buf), "Max: %.1f C", max_temp);
    lv_label_set_text(max_label_, max_buf);
    
    char min_buf[32];
    snprintf(min_buf, sizeof(min_buf), "Min: %.1f C", min_temp);
    lv_label_set_text(min_label_, min_buf);
    
    // Update waterline
    char wl_buf[32];
    snprintf(wl_buf, sizeof(wl_buf), "Avg: %.1f C", avg);
    lv_label_set_text(waterline_label_, wl_buf);
    
    // Calculate waterline Y position
    float norm = (avg - min_temp) / (max_temp - min_temp);
    int waterline_y = (int)(norm * 180 * TEMP_WATERLINE_Y_RATIO);
    lv_obj_align_to(temp_grid_h_, chart_bg_, LV_ALIGN_TOP_LEFT, 0, waterline_y);
}
