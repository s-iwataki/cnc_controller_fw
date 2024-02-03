#include "gui.h"
#include "ra_spi.h"
#include "spi.h"
#include "ui_thread.h"
#include "st7735.h"
#include "digitalout.h"
#include "hal_data.h"

#include "FreeRTOS.h"
#include "bsp_pin_cfg.h"
#include "command_list.h"
#include "common_data.h"
#include "portmacro.h"
#include "projdefs.h"
#include "r_ioport.h"
#include "task.h"
#include "timers.h"
#include <stdio.h>
/* user interface entry function */
/* pvParameters contains TaskHandle_t */
void ui_thread_entry(void* pvParameters) {
    FSP_PARAMETER_NOT_USED(pvParameters);
    DIGITALOUT_t lcd_rs={.pin=UI_LCD_A0};
    DIGITALOUT_t lcd_cs={.pin=UI_LCD_CS};
    DIGITALOUT_t lcd_reset={.pin=UI_LCD_RESET};
    printf("spi init\r\n");
    spi_bus_driver_t*drv=spi_init();
    printf("lcd init\r\n");

    gui_register_graphic_driver(st7735_init(ST7735R_18GREENTAB, drv, &lcd_reset, &lcd_cs, &lcd_rs));
    printf("lcd init ok\r\n");
    clear_screen();
    printf("cls end\r\n");
    /* TODO: add your own code here */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
