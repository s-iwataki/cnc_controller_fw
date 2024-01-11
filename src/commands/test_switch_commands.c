#include "test_switch_commands.h"

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
#include "r_ioport.h"
#include "subcmd_helper.h"
#include "task.h"
#include "timers.h"

typedef struct {
    const char* const label;
    bsp_io_port_pin_t pin;
} switch_entry_t;

switch_entry_t switchs[] = {
    {"x+ limit", XP_LIM},
    {"x- limit", XM_LIM},
    {"y+ limit", YP_LIM},
    {"y- limit", YM_LIM},
    {"z+ limit", ZP_LIM},
    {"z- limit", ZM_LIM},
    {"setzero", UI_SET_ORIG},
    {"spindle", UI_SPINDLE_ON},
    {"autofeed on", UI_AUTOFEED_ON},
    {"autofeed dir", UI_AUTOFEED_DIR},
    {"xsel", UI_XSEL},
    {"ysel", UI_YSEL},
    {"zsel", UI_ZSEL},
    {NULL, 0}};

static StaticTimer_t tm;
static void timer_callback(TimerHandle_t h) {
    printf("\033[2J\033[1;1H");
    for (int i = 0; switchs[i].label != NULL; i++) {
        bsp_io_level_t val;
        R_IOPORT_PinRead(&g_ioport_ctrl, switchs[i].pin, &val);
        printf("%s:%d ", switchs[i].label, val);
        if (i % 8 == 7) {
            printf("\r\n");
        }
    }
}

int test_switch_cmd(int argc, char** argv) {
    static TimerHandle_t report_timer = NULL;
    if (report_timer == NULL) {
        report_timer = xTimerCreateStatic("sw_report", pdMS_TO_TICKS(100), pdTRUE, NULL, timer_callback, &tm);
    }
    xTimerStart(report_timer, pdMS_TO_TICKS(1000));
    char dummy[2];
    fgets(dummy, sizeof(dummy), stdin);
    xTimerStop(report_timer, pdMS_TO_TICKS(1000));
    printf("\033[2J\033[1;1H");
    return 0;
}