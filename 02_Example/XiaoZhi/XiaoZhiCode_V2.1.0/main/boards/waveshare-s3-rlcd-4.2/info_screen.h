#ifndef INFO_SCREEN_H
#define INFO_SCREEN_H

#include <lvgl.h>
#include <esp_timer.h>
#include <esp_heap_caps.h>
#include <esp_wifi.h>
#include <esp_mac.h>

#define INFO_SCREEN_REFRESH_INTERVAL_S 30

class InfoScreen {
public:
    void Create(lv_obj_t* parent);
    void Refresh();

private:
    void UpdateNetwork();
    void UpdateSystem();
    void GetUptimeString(char* buf, size_t len);

    // Network labels
    lv_obj_t* ssid_label_ = nullptr;
    lv_obj_t* ip_label_ = nullptr;
    lv_obj_t* mac_label_ = nullptr;
    lv_obj_t* rssi_label_ = nullptr;

    // System labels
    lv_obj_t* battery_label_ = nullptr;
    lv_obj_t* charging_label_ = nullptr;
    lv_obj_t* uptime_label_ = nullptr;
    lv_obj_t* heap_label_ = nullptr;
    lv_obj_t* min_heap_label_ = nullptr;
    lv_obj_t* flash_label_ = nullptr;
    lv_obj_t* chip_label_ = nullptr;

    // Refresh timer
    esp_timer_handle_t refresh_timer_ = nullptr;
};

#endif // INFO_SCREEN_H
