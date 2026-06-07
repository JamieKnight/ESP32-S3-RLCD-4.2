/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef WIDGET_INIT_H
#define WIDGET_INIT_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "gui_guider.h"

/* Simple RTC time struct for C compilation (C++ includes i2c_equipment.h) */
#ifndef __cplusplus
typedef struct {
    int hour;
    int minute;
    int second;
} rtcTimeStruct_t;
#endif

__attribute__((unused)) void kb_event_cb(lv_event_t *e);
__attribute__((unused)) void ta_event_cb(lv_event_t *e);
__attribute__((unused)) void chart_update_temp_series(float temperature);
void update_info_tab(float temperature, rtcTimeStruct_t *timeData);
void switch_to_tab(uint8_t tab_id);
#if LV_USE_ANALOGCLOCK != 0
void clock_count(int *hour, int *min, int *sec);
#endif



#ifdef __cplusplus
}
#endif
#endif
