#include "test_adc_commands.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "bsp_pin_cfg.h"
#include "command_list.h"
#include "common_data.h"
#include "hal_data.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "r_adc.h"
#include "r_adc_api.h"
#include "r_ioport.h"
#include "subcmd_helper.h"
#include "task.h"
#include "timers.h"

static void adc0_cb(adc_callback_args_t* arg) {
    TaskHandle_t h = *(TaskHandle_t*)arg->p_context;
    BaseType_t wakeup;
    xTaskNotifyFromISR(h, 0x01, eSetBits, &wakeup);
    portYIELD_FROM_ISR(wakeup);
}
static void adc1_cb(adc_callback_args_t* arg) {
    TaskHandle_t h = *(TaskHandle_t*)arg->p_context;
    BaseType_t wakeup;
    xTaskNotifyFromISR(h, 0x02, eSetBits, &wakeup);
    portYIELD_FROM_ISR(wakeup);
}

int test_adc_cmd(int argc, char** argv) {
    uint16_t adc0_val, adc1_val;
    TaskHandle_t h = xTaskGetCurrentTaskHandle();
    g_adc0.p_api->callbackSet(g_adc0.p_ctrl, adc0_cb, &h, NULL);
    g_adc1.p_api->callbackSet(g_adc1.p_ctrl, adc1_cb, &h, NULL);
    TickType_t tick = xTaskGetTickCount();
    for (int i = 0; i < 100; i++) {
        xTaskDelayUntil(&tick, pdMS_TO_TICKS(100));
        g_adc0.p_api->scanStart(g_adc0.p_ctrl);
        g_adc1.p_api->scanStart(g_adc1.p_ctrl);
        uint32_t notification = 0;
        while ((notification & 0x03) != 0x03) {
            xTaskNotifyWait(0x03, 0x0, &notification, portMAX_DELAY);
        }
        ulTaskNotifyValueClear(NULL, 0x03);
        g_adc0.p_api->read(g_adc0.p_ctrl, ADC_CHANNEL_17, &adc0_val);
        g_adc1.p_api->read(g_adc1.p_ctrl, ADC_CHANNEL_17, &adc1_val);
        printf("adc0=%d, adc1=%d \r\n", adc0_val, adc1_val);
    }

    return 0;
}