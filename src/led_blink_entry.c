#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "bsp_pin_cfg.h"
#include "hal_data.h"
#include "led_blink.h"
#include "led_indicator.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "r_ioport.h"
#include "r_sci_uart.h"

/* ledblink entry function */
/* pvParameters contains TaskHandle_t */

typedef struct {
    led_indicator_pattern_t pattern;
    bsp_io_level_t green_level;
    bsp_io_level_t red_level;
    QueueHandle_t q;
    TickType_t wait;
} led_indicator_state_t;

#define LED_INDICATOR_CMD_QUEUE_SIZE 1
#define LED_INDICATOR_CMD_QUEUE_ITEM_SZ sizeof(led_indicator_pattern_t)
static StaticQueue_t led_indiactor_cmd_queue;
uint8_t led_indiactor_cmd_queue_data[LED_INDICATOR_CMD_QUEUE_ITEM_SZ * LED_INDICATOR_CMD_QUEUE_SIZE];
static led_indicator_state_t led_indicator_state = {0};

void led_blink_entry(void* pvParameters) {
    FSP_PARAMETER_NOT_USED(pvParameters);
    /* TODO: add your own code here */
    led_indicator_state.pattern = LED_INDICATE_STANDBY;
    led_indicator_state.wait = portMAX_DELAY;
    led_indicator_state.q = xQueueCreateStatic(LED_INDICATOR_CMD_QUEUE_SIZE, LED_INDICATOR_CMD_QUEUE_ITEM_SZ, led_indiactor_cmd_queue_data, &led_indiactor_cmd_queue);
    while (1) {
        /*R_IOPORT_PinWrite(&g_ioport_ctrl, LED_GREEN, BSP_IO_LEVEL_HIGH);
        R_IOPORT_PinWrite(&g_ioport_ctrl, LED_RED, BSP_IO_LEVEL_HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));
        R_IOPORT_PinWrite(&g_ioport_ctrl, LED_GREEN, BSP_IO_LEVEL_LOW);
        R_IOPORT_PinWrite(&g_ioport_ctrl, LED_RED, BSP_IO_LEVEL_LOW);
        vTaskDelay(pdMS_TO_TICKS(500));*/
        switch (led_indicator_state.pattern) {
            case LED_INDICATE_STANDBY:
                led_indicator_state.wait = portMAX_DELAY;
                led_indicator_state.green_level = BSP_IO_LEVEL_HIGH;
                led_indicator_state.red_level = BSP_IO_LEVEL_LOW;
                break;
            case LED_INDICATE_RUN:
                led_indicator_state.wait = pdMS_TO_TICKS(500);
                if (led_indicator_state.green_level == BSP_IO_LEVEL_HIGH) {
                    led_indicator_state.green_level = BSP_IO_LEVEL_LOW;
                } else {
                    led_indicator_state.green_level = BSP_IO_LEVEL_HIGH;
                }
                led_indicator_state.red_level = BSP_IO_LEVEL_LOW;
                break;
            case LED_INDICATE_ERROR:
                led_indicator_state.wait = pdMS_TO_TICKS(200);
                if (led_indicator_state.red_level == BSP_IO_LEVEL_HIGH) {
                    led_indicator_state.red_level = BSP_IO_LEVEL_LOW;
                } else {
                    led_indicator_state.red_level = BSP_IO_LEVEL_HIGH;
                }
                led_indicator_state.green_level = BSP_IO_LEVEL_LOW;
                break;
            default:
                break;
        }
        led_indicator_pattern_t s = LED_INDICATE_STANDBY;
        R_IOPORT_PinWrite(&g_ioport_ctrl, LED_GREEN, led_indicator_state.green_level);
        R_IOPORT_PinWrite(&g_ioport_ctrl, LED_RED, led_indicator_state.red_level);
        if (xQueueReceive(led_indicator_state.q, &s, led_indicator_state.wait)) {
            led_indicator_state.pattern = s;
        }
    }
}

void led_indicator_set(led_indicator_pattern_t pattern) {
    if (led_indicator_state.q == 0) {
        return;  // not ready
    }
    xQueueSendToBack(led_indicator_state.q, &pattern, portMAX_DELAY);
}
