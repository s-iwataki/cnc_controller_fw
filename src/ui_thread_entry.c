#include "graphic.h"
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
#include "ui_view.h"




/* user interface entry function */
/* pvParameters contains TaskHandle_t */
void ui_thread_entry(void* pvParameters) {
    FSP_PARAMETER_NOT_USED(pvParameters);
    DIGITALOUT_t lcd_rs={.pin=UI_LCD_A0};
    DIGITALOUT_t lcd_cs={.pin=UI_LCD_CS};
    DIGITALOUT_t lcd_reset={.pin=UI_LCD_RESET};
    gui_register_graphic_driver(st7735_init(ST7735R_18BLACKTAB, spi_init(), &lcd_reset, &lcd_cs, &lcd_rs));
    const graphic_apis_t*api=gui_get_graphic_driver();
    api->set_rotation(api,3);
    clear_screen();
    ui_view_init();
    /* TODO: add your own code here */
    TickType_t last_wake_time = xTaskGetTickCount();
    
    while (1) {
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(UI_VIEW_UPDATE_PERIOD_MS));
        ui_view_process();
    }
}
