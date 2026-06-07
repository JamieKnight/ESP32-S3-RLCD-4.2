#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <esp_log.h>
#include "button_bsp.h"
#include "user_app.h"
#include "gui_guider.h"
#include "i2c_equipment.h"
#include "i2c_bsp.h"
#include "sdcard_bsp.h"
#include "codec_bsp.h"
#include "adc_bsp.h"
#include "esp_wifi_bsp.h"
#include "ble_scan_bsp.h"
#include "widgets_init.h"

/* Global UI object (defined in gui_guider.h as extern) */
lv_ui init_ui;
lv_coord_t temp_history[60];
uint8_t temp_idx = 0;
bool temp_initialized = false;

I2cMasterBus I2cbus(14,13,0);
CustomSDPort *sdcardPort = NULL;
Shtc3Port *shtc3port = NULL;
EventGroupHandle_t CodecGroups;
CodecPort *codecport = NULL;
static uint8_t *audio_ptr = NULL;
static bool is_Music = true;

void chart_update_temp_series(float temperature);

void Lvgl_UserTask(void *arg) {
    uint32_t times = 0;
    uint32_t shtc3_time = 0;
    float rh = 0, temp = 0;
    rtcTimeStruct_t timerData;

    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(200));
        times++;

        /* Read SHTC3 every 25 ticks (5 seconds) */
        if(times - shtc3_time == 25) {
            shtc3_time = times;
            shtc3port->Shtc3_ReadTempHumi(&temp,&rh);

            /* Update chart with scaled temperature */
            chart_update_temp_series(temp);

            /* Update Info tab labels if Info tab is active */
            uint8_t current_tab = lv_tabview_get_tab_act(init_ui.screen_tabview);
            if (current_tab == 1) {
                Rtc_GetTime(&timerData);
                update_info_tab(temp, &timerData);
            }
        }
    }
}

void BOOT_LoopTask(void *arg) {
    for(;;) {
        EventBits_t even = xEventGroupWaitBits(BootButtonGroups,(0x01 | 0x02 | 0x04),pdTRUE,pdFALSE,pdMS_TO_TICKS(2000));
        if(even & 0x01) {
            xEventGroupSetBits(CodecGroups,0x02);
        } else if(even & 0x02) {
            xEventGroupSetBits(CodecGroups,0x01);
        }
    }
}

void KEY_LoopTask(void *arg) {
    for(;;) {
        EventBits_t even = xEventGroupWaitBits(GP18ButtonGroups,(0x01 | 0x02 | 0x04),pdTRUE,pdFALSE,pdMS_TO_TICKS(2000));
        if(even & 0x01) {
            /* Single click: switch tabs */
            uint8_t current = lv_tabview_get_tab_act(init_ui.screen_tabview);
            uint8_t next = (current == 0) ? 1 : 0;
            lv_tabview_set_act(init_ui.screen_tabview, next, LV_ANIM_ON);
        }
    }
}

void Codec_LoopTask(void *arg) {
    bool is_eco = 0;
    for(;;) {
        EventBits_t even = xEventGroupWaitBits(CodecGroups,(0x01 | 0x02 | 0x04),pdTRUE,pdFALSE,pdMS_TO_TICKS(8 * 1000));
        if(even & 0x01) {
            codecport->CodecPort_EchoRead(audio_ptr,192 * 1000);
            is_eco = 1;
        }
        else if(even & 0x02) {
            if(1 == is_eco) {
                is_eco = 0;
                codecport->CodecPort_PlayWrite(audio_ptr,192 * 1000);
            }
        }
        else if(even & 0x04) {
            codecport->CodecPort_SetSpeakerVol(90);
            uint32_t bytes_sizt;
            size_t bytes_write = 0;
            uint8_t *data_ptr = codecport->CodecPort_GetPcmData(&bytes_sizt);
            while (bytes_write < bytes_sizt) {
                codecport->CodecPort_PlayWrite(data_ptr, 256);
                data_ptr += 256;
                bytes_write += 256;
                if(!is_Music) break;
            }
            codecport->CodecPort_SetSpeakerVol(100);
        }
    }
}

void UserApp_AppInit() {
    audio_ptr = (uint8_t *)heap_caps_malloc(288 * 1000 * sizeof(uint8_t), MALLOC_CAP_SPIRAM);
    assert(audio_ptr);
    sdcardPort = new CustomSDPort("/sdcard");
    Adc_PortInit();
    Custom_ButtonInit();
    Rtc_Setup(&I2cbus,0x51);
    Rtc_SetTime(2026,6,1,12,29,24);
    shtc3port = new Shtc3Port(I2cbus);
    espwifi_init();
    CodecGroups = xEventGroupCreate();
    codecport = new CodecPort(I2cbus,"S3_RLCD_4_2");
    codecport->CodecPort_SetInfo("es8311 & es7210",1,16000,2,16);
    codecport->CodecPort_SetSpeakerVol(100);
    codecport->CodecPort_SetMicGain(35);
}

void UserApp_UiInit() {
    setup_ui(&init_ui);
}

void UserApp_TaskInit() {
    xTaskCreatePinnedToCore(Lvgl_UserTask, "Lvgl_UserTask", 5 * 1024, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(BOOT_LoopTask, "BOOT_LoopTask", 4 * 1024, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(KEY_LoopTask, "KEY_LoopTask", 4 * 1024, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(Codec_LoopTask, "Codec_LoopTask", 4 * 1024, NULL, 2, NULL, 1);
}
