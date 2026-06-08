#include "info_screen.h"
#include "wifi_station.h"
#include "board.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <esp_wifi.h>
#include <esp_mac.h>

#define TAG "info_screen"

// Get human-readable uptime
void InfoScreen::GetUptimeString(char* buf, size_t len) {
    uint64_t us = esp_timer_get_time();
    uint64_t s = us / 1000000;
    int days = s / 86400;
    int hours = (s % 86400) / 3600;
    int mins = (s % 3600) / 60;
    int secs = s % 60;
    if (days > 0) {
        snprintf(buf, len, "%dd %02d:%02d:%02d", days, hours, mins, secs);
    } else {
        snprintf(buf, len, "%02d:%02d:%02d", hours, mins, secs);
    }
}

void InfoScreen::Create(lv_obj_t* parent) {
    ESP_LOGI(TAG, "Creating info screen");
    
    // Title
    lv_obj_t* title = lv_label_create(parent);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, lv_font_dejavu_16_persian_ansi, LV_PART_MAIN);
    lv_label_set_text(title, "\u2139 System Info");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 8, 15);
    (void)title;  // not stored, title is static
    
    // WiFi Section
    ssid_label_ = lv_label_create(parent);
    lv_label_set_text(ssid_label_, "SSID: Connecting...");
    lv_obj_set_style_text_font(ssid_label_, lv_font_dejavu_14_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(ssid_label_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align(ssid_label_, LV_ALIGN_TOP_LEFT, 8, 45);
    
    ip_label_ = lv_label_create(parent);
    lv_label_set_text(ip_label_, "IP: ---");
    lv_obj_set_style_text_font(ip_label_, lv_font_dejavu_14_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(ip_label_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align_to(ip_label_, ssid_label_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);
    
    mac_label_ = lv_label_create(parent);
    lv_obj_set_style_text_font(mac_label_, lv_font_dejavu_14_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(mac_label_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align_to(mac_label_, ip_label_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);
    lv_label_set_text(mac_label_, "MAC: ---");
    
    rssi_label_ = lv_label_create(parent);
    lv_obj_set_style_text_font(rssi_label_, lv_font_dejavu_14_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(rssi_label_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align_to(rssi_label_, mac_label_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);
    lv_label_set_text(rssi_label_, "RSSI: ---");
    
    // Divider
    lv_obj_t* divider1 = lv_line_create(parent);
    static lv_point_t pts1[] = { {0, 5}, {390, 5} };
    lv_line_set_points(divider1, pts1, 2);
    lv_obj_set_style_line_color(divider1, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_set_style_line_width(divider1, 1, LV_PART_MAIN);
    lv_obj_align_to(divider1, rssi_label_, LV_ALIGN_OUT_BOTTOM_LEFT, -8, 12);
    
    // Battery Section
    battery_label_ = lv_label_create(parent);
    lv_label_set_text(battery_label_, "Battery: ---%");
    lv_obj_set_style_text_font(battery_label_, lv_font_dejavu_14_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(battery_label_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align_to(battery_label_, divider1, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    
    charging_label_ = lv_label_create(parent);
    lv_label_set_text(charging_label_, "Charging: No");
    lv_obj_set_style_text_font(charging_label_, lv_font_dejavu_14_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(charging_label_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align_to(charging_label_, battery_label_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);
    
    // System Section
    lv_obj_t* divider2 = lv_line_create(parent);
    static lv_point_t pts2[] = { {0, 5}, {390, 5} };
    lv_line_set_points(divider2, pts2, 2);
    lv_obj_set_style_line_color(divider2, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_set_style_line_width(divider2, 1, LV_PART_MAIN);
    lv_obj_align_to(divider2, charging_label_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    
    uptime_label_ = lv_label_create(parent);
    lv_label_set_text(uptime_label_, "Uptime: ---");
    lv_obj_set_style_text_font(uptime_label_, lv_font_dejavu_14_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(uptime_label_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align_to(uptime_label_, divider2, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    
    heap_label_ = lv_label_create(parent);
    lv_obj_set_style_text_font(heap_label_, lv_font_dejavu_14_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(heap_label_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align_to(heap_label_, uptime_label_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);
    lv_label_set_text(heap_label_, "Heap: ---");
    
    min_heap_label_ = lv_label_create(parent);
    lv_obj_set_style_text_font(min_heap_label_, lv_font_dejavu_14_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(min_heap_label_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align_to(min_heap_label_, heap_label_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);
    lv_label_set_text(min_heap_label_, "Min Heap: ---");
    
    flash_label_ = lv_label_create(parent);
    lv_obj_set_style_text_font(flash_label_, lv_font_dejavu_14_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(flash_label_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align_to(flash_label_, min_heap_label_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);
    lv_label_set_text(flash_label_, "Flash: ---");
    
    chip_label_ = lv_label_create(parent);
    lv_obj_set_style_text_font(chip_label_, lv_font_dejavu_14_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(chip_label_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align_to(chip_label_, flash_label_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);
    
    esp_err_t chip_rev;
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    chip_label_ = lv_label_create(parent);
    lv_label_set_text_fmt(chip_label_, "Chip: ESP32-S3 Rev %d", chip_info.revision);
    lv_obj_set_style_text_font(chip_label_, lv_font_dejavu_14_persian_ansi, LV_PART_MAIN);
    lv_obj_set_style_text_color(chip_label_, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align_to(chip_label_, flash_label_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);
    
    // Refresh timer (every 30 seconds)
    esp_timer_create_args_t timer_args = {
        .callback = [](void* arg) {
            InfoScreen* self = static_cast<InfoScreen*>(arg);
            if (self) {
                self->Refresh();
            }
        },
        .arg = this,
        .name = "info_refresh"
    };
    esp_timer_create(&timer_args, &refresh_timer_);
    esp_timer_start_periodic(refresh_timer_, INFO_SCREEN_REFRESH_INTERVAL_S * 1000000ULL);
}

void InfoScreen::UpdateNetwork() {
    auto& station = WifiStation::GetInstance();
    station.Refresh();
    
    const char* ssid = station.GetSsid().c_str();
    char ssid_buf[64];
    snprintf(ssid_buf, sizeof(ssid_buf), "SSID: %s", ssid);
    lv_label_set_text(ssid_label_, ssid_buf);
    
    char ip_buf[32];
    snprintf(ip_buf, sizeof(ip_buf), "IP: %s", station.GetIpAddress().c_str());
    lv_label_set_text(ip_label_, ip_buf);
    
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char mac_buf[32];
    snprintf(mac_buf, sizeof(mac_buf), "MAC: %02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    lv_label_set_text(mac_label_, mac_buf);
    
    int16_t rssi = 0;
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        rssi = ap_info.rssi;
    }
    char rssi_buf[32];
    snprintf(rssi_buf, sizeof(rssi_buf), "RSSI: %d dBm", rssi);
    lv_label_set_text(rssi_label_, rssi_buf);
}

void InfoScreen::UpdateSystem() {
    // Battery
    int battery_level = 0;
    bool charging = false, discharging = false;
    Board::GetInstance().GetBatteryLevel(battery_level, charging, discharging);
    
    char batt_buf[32];
    snprintf(batt_buf, sizeof(batt_buf), "Battery: %d%%", battery_level);
    lv_label_set_text(battery_label_, batt_buf);
    
    char charge_buf[32];
    snprintf(charge_buf, sizeof(charge_buf), "Charging: %s", charging ? "Yes" : "No");
    lv_label_set_text(charging_label_, charge_buf);
    
    // Uptime
    char uptime_buf[32];
    GetUptimeString(uptime_buf, sizeof(uptime_buf));
    lv_label_set_text(uptime_label_, uptime_buf);
    
    // Heap
    size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    char heap_buf[64];
    snprintf(heap_buf, sizeof(heap_buf), "Heap: %lu / %lu", free_heap, ESP_ISRAM ? 0 : heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    lv_label_set_text(heap_label_, heap_buf);
    
    // Minimum ever-free heap
    char min_heap_buf[32];
    snprintf(min_heap_buf, sizeof(min_heap_buf), "Min Heap: %lu", esp_get_minimum_free_heap_size());
    lv_label_set_text(min_heap_label_, min_heap_buf);
    
    // Flash size
    char flash_buf[32];
    snprintf(flash_buf, sizeof(flash_buf), "Flash: %lu MB", 16);  // Default 16MB for ESP32-S3
    lv_label_set_text(flash_label_, flash_buf);
}

void InfoScreen::Refresh() {
    UpdateNetwork();
    UpdateSystem();
}
