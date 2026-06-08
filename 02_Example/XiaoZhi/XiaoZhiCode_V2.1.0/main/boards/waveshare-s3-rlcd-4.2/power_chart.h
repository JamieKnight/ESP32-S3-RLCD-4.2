#ifndef POWER_CHART_H
#define POWER_CHART_H

#include <lvgl.h>
#include <esp_timer.h>
#include <cJSON.h>
#include <string>
#include <vector>

#define SHELLY_API_PORT 8083
#define SHELLY_API_PATH "/api/charts"
#define SHELLY_API_QUERY "?window=hourly"
#define CHART_POINTS 24  // 24 hourly data points

class PowerChart {
public:
    PowerChart();
    ~PowerChart();

    // Initialize with the tabview handle
    void Initialize(lv_obj_t* tabview);
    
    // Create chart objects in the power tab
    void Create(lv_obj_t* power_tab);
    
    // Show the chart and start fetching data
    void Show();
    
    // Fetch data from Shelly API (called by background task)
    bool ParseAndRender(const char* json_response, int length);
    
    // Fetch data from Shelly API
    void FetchData();
    
    // Render chart bars from data
    void RenderChart();
    
    // Get status label for external updates
    lv_obj_t* GetStatusLabel() const;
    
    // Get current IP address of the device
    std::string GetLocalIP();

private:
    lv_obj_t* tabview_ = nullptr;
    lv_obj_t* chart_tab_ = nullptr;
    lv_obj_t* chart_background_ = nullptr;
    lv_obj_t* title_label_ = nullptr;
    lv_obj_t* value_label_ = nullptr;
    lv_obj_t* status_label_ = nullptr;
    
    // Chart area dimensions
    static constexpr int CHART_X = 20;
    static constexpr int CHART_Y = 60;
    static constexpr int CHART_W = 360;
    static constexpr int CHART_H = 180;
    static constexpr int BAR_WIDTH = 10;
    static constexpr int BAR_GAP = 4;
    static constexpr int Y_LABEL_SPACE = 55;
    
    // Data
    double hourly_data_[CHART_POINTS] = {};
    int data_count_ = 0;
    bool data_fetched_ = false;
    
    // Timer for periodic refresh
    esp_timer_handle_t refresh_timer_ = nullptr;
    
    // Bar object handles for rendering
    std::vector<lv_obj_t*> bar_objects_;
    
    // Format power value
    std::string FormatPower(double watts);
    
    // Draw chart axes
    void DrawAxes();
    
    // Update stats label
    void UpdateStats();
};

#endif // POWER_CHART_H
