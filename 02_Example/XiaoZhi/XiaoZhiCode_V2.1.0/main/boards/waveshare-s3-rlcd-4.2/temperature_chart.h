#ifndef TEMPERATURE_CHART_H
#define TEMPERATURE_CHART_H

#include <lvgl.h>
#include <esp_timer.h>
#include <driver/temperature_sensor.h>

#define TEMP_SAMPLE_INTERVAL_S 5
#define TEMP_BUFFER_SIZE 288    // 5 min intervals for 24 hours
#define TEMP_WATERLINE_Y_RATIO 0.75

class TemperatureChart {
public:
    void Create(lv_obj_t* parent);
    void Show();
    
private:
    void StartPolling();
    void StopPolling();
    void OnTemperatureSample();
    
    void RenderChart();
    void UpdateStats();
    
    // LVGL objects
    lv_obj_t* title_label_ = nullptr;
    lv_obj_t* chart_bg_ = nullptr;
    lv_obj_t* waterline_label_ = nullptr;
    lv_obj_t* current_label_ = nullptr;
    lv_obj_t* avg_label_ = nullptr;
    lv_obj_t* max_label_ = nullptr;
    lv_obj_t* min_label_ = nullptr;
    lv_obj_t* refresh_label_ = nullptr;
    lv_obj_t* temp_grid_h_ = nullptr;  // Horizontal grid line
    lv_obj_t* bars_[TEMP_BUFFER_SIZE] = {};
    
    // Data
    float temp_buffer_[TEMP_BUFFER_SIZE] = {};
    int buffer_count_ = 0;
    int write_index_ = 0;
    bool has_data_ = false;
    
    // Temperature sensor
    temperature_sensor_handle_t temp_sensor_ = NULL;
    esp_timer_handle_t sample_timer_ = nullptr;
};

#endif // TEMPERATURE_CHART_H
