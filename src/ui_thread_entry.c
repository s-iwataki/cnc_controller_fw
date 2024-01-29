#include "gui.h"
#include "ra_spi.h"
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
/* user interface entry function */
/* pvParameters contains TaskHandle_t */
void ui_thread_entry(void* pvParameters) {
    FSP_PARAMETER_NOT_USED(pvParameters);
    DIGITALOUT_t lcd_rs={.pin=UI_LCD_A0};
    DIGITALOUT_t lcd_cs={.pin=UI_LCD_CS};
    DIGITALOUT_t lcd_reset={.pin=UI_LCD_RESET};

    gui_register_graphic_driver(st7735_init(ST7735R_18GREENTAB, spi_init(), &lcd_reset, &lcd_cs, &lcd_rs));

    /* TODO: add your own code here */
    while (1) {
        vTaskDelay(1);
    }
}
