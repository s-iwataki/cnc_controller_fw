#include "test_spindle_commands.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "bsp_pin_cfg.h"
#include "command_list.h"
#include "commands/subcmd_helper.h"
#include "common_data.h"
#include "hal_data.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "r_ioport.h"
#include "spindle.h"
#include "subcmd_helper.h"
#include "task.h"
#include "timers.h"
#include "utils.h"

static StaticTimer_t tm;
static void timer_callback(TimerHandle_t h) {
    int rpm = spindle_get_speed(&g_spindle_motor);
    printf("\033[2J\033[1;1H %d rpm", rpm);
}

static void print_rpm_and_wait_key(TimerHandle_t h) {
    spindle_enable(&g_spindle_motor, pdTRUE);
    xTimerStart(h, pdMS_TO_TICKS(1000));
    char dummy[2];
    fgets(dummy, sizeof(dummy), stdin);
    xTimerStop(h, pdMS_TO_TICKS(1000));
    printf("\033[2J\033[1;1H");
    spindle_enable(&g_spindle_motor, pdFALSE);
}

static int set_param_cmd(int argc, char** argv) {
    if (argc != 4) {
        printf("usage: set_parameter kp ki kd\r\n");
        return -1;
    }
    float kp, ki, kd;
    if (my_atof(argv[1], &kp) < 0) {
        printf("parse error\r\n");
        return -1;
    }
    if (my_atof(argv[2], &ki) < 0) {
        printf("parse error\r\n");
        return -1;
    }

    if (my_atof(argv[3], &kd) < 0) {
        printf("parse error\r\n");
        return -1;
    }
    spindle_set_control_param(&g_spindle_motor, kp, ki, kd);
    return 0;
}
static int set_duty_cmd(int argc, char** argv) {
    int target_duty = atoi(argv[1]);
    spindle_set_duty(&g_spindle_motor, target_duty);
    return 1;
}
static int set_speed_cmd(int argc, char** argv) {
    int target_speed = atoi(argv[1]);
    spindle_set_speed(&g_spindle_motor, target_speed);
    return 1;
}
static int get_speed_cmd(int argc, char** argv) {
    return 1;
}

static command_entry_t subcmd[] = {
    {"set_param", set_param_cmd, "set speed control parameter"},
    {"set_duty", set_duty_cmd, "set motor pwm duty"},
    {"set_duty", set_speed_cmd, "set target speed"},
    {"get_speed", get_speed_cmd, "get currenct speed"},
    {NULL, NULL, NULL}};

int test_spindle_cmd(int argc, char** argv) {
    static TimerHandle_t report_timer = NULL;
    if (report_timer == NULL) {
        spindle_init(&g_spindle_motor);
        report_timer = xTimerCreateStatic("spindle_report", pdMS_TO_TICKS(100), pdTRUE, NULL, timer_callback, &tm);
    }
    int retval=apply_sub_cmd(argc, argv, subcmd);
    if(retval>0){
        print_rpm_and_wait_key(report_timer);
    }
    if(retval>0){
        retval=0;
    }
    return retval;
}