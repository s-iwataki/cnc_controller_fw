#include "test_encoder_commands.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "bsp_pin_cfg.h"
#include "command_list.h"
#include "common_data.h"
#include "encoder.h"
#include "hal_data.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "r_ioport.h"
#include "subcmd_helper.h"
#include "task.h"
#include "timers.h"

static StaticTimer_t tm;
static void timer_callback(TimerHandle_t h) {
    update_encoder(&g_ui_encoder);
    int diff = get_encoder_diff(&g_ui_encoder);
    int val = get_encoder_val(&g_ui_encoder);
    printf("\033[2J\033[1;1H encoder val:%d diff:%d\r\n press any key.", val, diff);
}

int test_encoder_cmd(int argc, char** argv) {
    static TimerHandle_t report_timer = NULL;
    if (report_timer == NULL) {
        report_timer = xTimerCreateStatic("enc_report", pdMS_TO_TICKS(100), pdTRUE, NULL, timer_callback, &tm);
    }
    xTimerStart(report_timer, pdMS_TO_TICKS(1000));
    char dummy[2];
    fgets(dummy, sizeof(dummy), stdin);
    xTimerStop(report_timer, pdMS_TO_TICKS(1000));
    printf("\033[2J\033[1;1H");
    return 0;
}