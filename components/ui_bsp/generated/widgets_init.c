/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include "gui_guider.h"
#include "widgets_init.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>

/* ======== Chart update ======== */
void chart_update_temp_series(float temperature) {
    /* Scale to 10x: 15°C → 150, 34°C → 340 */
    lv_coord_t scaled = (lv_coord_t)(temperature * 10);
    if (scaled < 150) scaled = 150;
    if (scaled > 340) scaled = 340;

    temp_history[temp_idx] = scaled;
    temp_idx = (temp_idx + 1) % 60;

    if (!temp_initialized) {
        /* Fill entire buffer with first reading so chart isn't flatline */
        for (int i = 0; i < 60; i++) {
            temp_history[i] = scaled;
        }
        lv_chart_set_ext_y_array(init_ui.screen_chart, init_ui.screen_chart_series, temp_history);
        temp_initialized = true;
    } else {
        /* Update only the latest point */
        lv_chart_set_ext_y_array(init_ui.screen_chart, init_ui.screen_chart_series, temp_history);
    }
    lv_chart_refresh(init_ui.screen_chart);
}

/* ======== Tab switching via single-press on USER button (GPIO 18) ======== */
void switch_to_tab(uint8_t tab_id) {
    lv_tabview_set_act(init_ui.screen_tabview, tab_id, LV_ANIM_ON);
}

__attribute__((unused)) void kb_event_cb (lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *kb = lv_event_get_target(e);
    if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}

__attribute__((unused)) void ta_event_cb (lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
#if LV_USE_KEYBOARD || LV_USE_ZH_KEYBOARD
    lv_obj_t *ta = lv_event_get_target(e);
#endif
    lv_obj_t *kb = lv_event_get_user_data(e);
    if (code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED)
    {
#if LV_USE_ZH_KEYBOARD != 0
        lv_zh_keyboard_set_textarea(kb, ta);
#endif
#if LV_USE_KEYBOARD != 0
        lv_keyboard_set_textarea(kb, ta);
#endif
        lv_obj_move_foreground(kb);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
    if (code == LV_EVENT_CANCEL || code == LV_EVENT_DEFOCUSED)
    {

#if LV_USE_ZH_KEYBOARD != 0
        lv_zh_keyboard_set_textarea(kb, ta);
#endif
#if LV_USE_KEYBOARD != 0
        lv_keyboard_set_textarea(kb, ta);
#endif
        lv_obj_move_background(kb);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}

/* ======== Update Info tab: temp and time labels ======== */
void update_info_tab(float temperature, rtcTimeStruct_t *timeData) {
    char buf[16];
    /* Temperature with 1 decimal place: "23.4°" */
    snprintf(buf, sizeof(buf), "%.1f°", temperature);
    lv_label_set_text(init_ui.screen_label_temp, buf);

    /* Time in HH:MM:SS */
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", timeData->hour, timeData->minute, timeData->second);
    lv_label_set_text(init_ui.screen_label_time, buf);
}

#if LV_USE_ANALOGCLOCK != 0
void clock_count(int *hour, int *min, int *sec)
{
    (*sec)++;
    if(*sec == 60)
    {
        *sec = 0;
        (*min)++;
    }
    if(*min == 60)
    {
        *min = 0;
        if(*hour < 12)
        {
            (*hour)++;
        } else {
            (*hour)++;
            *hour = *hour %12;
        }
    }
}
#endif


