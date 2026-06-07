/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"


void setup_scr_screen(lv_ui *ui)
{
    //Write codes screen
    ui->screen = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen, 400, 300);
    lv_obj_set_scrollbar_mode(ui->screen, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_cont_1 (white background container)
    ui->screen_cont_1 = lv_obj_create(ui->screen);
    lv_obj_set_pos(ui->screen_cont_1, 0, 0);
    lv_obj_set_size(ui->screen_cont_1, 400, 300);
    lv_obj_set_scrollbar_mode(ui->screen_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_cont_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_cont_1, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_cont_1, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_cont_1, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_cont_1, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_tabview
    ui->screen_tabview = lv_tabview_create(ui->screen_cont_1, LV_DIR_TOP, 40);
    lv_obj_set_style_pad_top(ui->screen_tabview, 0, LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_tabview, 0, LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_tabview, 0, LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_tabview, 0, LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_tabview, lv_color_hex(0xffffff), LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_tabview, 255, LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_tabview, LV_GRAD_DIR_NONE, LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_tabview, 0, LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_tabview, 0, LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_tabview, 0, LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_tabview, &lv_font_MISANSMEDIUM_20, LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_tabview, lv_color_hex(0x000000), LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_tabview, 0, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_tabview, lv_color_hex(0x000000), LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_tabview, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_tabview, 0, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_tabview, 0, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_tabview, 0, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_tabview, 3, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_tabview, 0, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_tabview, 0, LV_PART_INDICATOR|LV_STATE_DEFAULT);

    //Add "Chart" tab
    ui->screen_chart_tab = lv_tabview_add_tab(ui->screen_tabview, "Chart");
    //Write style for Chart tab content
    lv_obj_set_style_pad_top(ui->screen_chart_tab, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_chart_tab, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_chart_tab, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_chart_tab, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_chart_tab, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_chart
    ui->screen_chart = lv_chart_create(ui->screen_chart_tab);
    lv_obj_set_size(ui->screen_chart, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(ui->screen_chart, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_chart, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_chart, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_chart, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_width(ui->screen_chart, 3, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_chart_set_type(ui->screen_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(ui->screen_chart, 60);
    lv_chart_set_range(ui->screen_chart, LV_CHART_AXIS_PRIMARY_Y, 150, 340);
    lv_chart_set_update_mode(ui->screen_chart, LV_CHART_UPDATE_MODE_SHIFT);

    //Write codes screen_chart_series
    ui->screen_chart_series = lv_chart_add_series(ui->screen_chart, lv_color_black(), LV_CHART_AXIS_PRIMARY_Y);

    // Y-axis: labels every 5°C (150-340), auto-generated from range
    lv_chart_set_axis_tick(ui->screen_chart, LV_CHART_AXIS_PRIMARY_Y, 3, 0, 10, 0, true, 25);
    // X-axis: 6 labels at 5-min intervals, auto-generated from range
    lv_chart_set_axis_tick(ui->screen_chart, LV_CHART_AXIS_PRIMARY_X, 3, 0, 6, 0, true, 15);

    //Write codes screen_info_tab
    ui->screen_info_tab = lv_tabview_add_tab(ui->screen_tabview, "Info");
    //Write style for Info tab content
    lv_obj_set_style_pad_top(ui->screen_info_tab, 16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_info_tab, 16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_info_tab, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_info_tab, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_info_tab, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_label_temp (left half - temperature)
    ui->screen_label_temp = lv_label_create(ui->screen_info_tab);
    lv_label_set_text(ui->screen_label_temp, "--°C");
    lv_label_set_long_mode(ui->screen_label_temp, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(ui->screen_label_temp, lv_pct(50));
    lv_obj_set_height(ui->screen_label_temp, lv_pct(100));
    lv_obj_set_style_text_color(ui->screen_label_temp, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_temp, &lv_font_MISANSMEDIUM_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_temp, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_label_time (right half - time)
    ui->screen_label_time = lv_label_create(ui->screen_info_tab);
    lv_label_set_text(ui->screen_label_time, "--:--");
    lv_label_set_long_mode(ui->screen_label_time, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(ui->screen_label_time, lv_pct(50));
    lv_obj_set_height(ui->screen_label_time, lv_pct(100));
    lv_obj_set_pos(ui->screen_label_time, lv_pct(50), 0);
    lv_obj_set_style_text_color(ui->screen_label_time, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_time, &lv_font_MISANSMEDIUM_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_time, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_label_active_tab (underline indicator for active tab)
    // Hidden by default, will be positioned dynamically

    //The custom code of screen.
    custom_init(ui);


    //Update current screen layout.
    lv_obj_update_layout(ui->screen);

}
