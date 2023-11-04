#include "bsp_pin_cfg.h"
#include "hal_data.h"
#include "led_blink.h"
#include "r_ioport.h"
#include "r_sci_uart.h"
#include <string.h>
#include <stdio.h>
/* ledblink entry function */
/* pvParameters contains TaskHandle_t */
const char*test_msg="test\r\n";
void led_blink_entry(void *pvParameters) {
  FSP_PARAMETER_NOT_USED(pvParameters);
  /* TODO: add your own code here */
  while (1) {
    R_IOPORT_PinWrite(&g_ioport_ctrl, LED_GREEN, BSP_IO_LEVEL_HIGH);
    R_IOPORT_PinWrite(&g_ioport_ctrl, LED_RED, BSP_IO_LEVEL_HIGH);
    vTaskDelay(pdMS_TO_TICKS(500));
    R_IOPORT_PinWrite(&g_ioport_ctrl, LED_GREEN, BSP_IO_LEVEL_LOW);
    R_IOPORT_PinWrite(&g_ioport_ctrl, LED_RED, BSP_IO_LEVEL_LOW);
    vTaskDelay(pdMS_TO_TICKS(500));
    printf("printf test\r\n");
  }
}
