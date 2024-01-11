
#include <stdint.h>
#include "FreeRTOS.h"
#include "bsp_pin_cfg.h"
#include "common_data.h"
#include "hal_data.h"
#include "portmacro.h"
#include "projdefs.h"
#include "r_gpt.h"
#include "r_ioport.h"
#include "r_timer_api.h"
#include "spindle.h"
#include "spindle_control_task.h"


spindle_motor_t g_spindle_motor;

void tmr_callback(timer_callback_args_t*arg){
    spindle_motor_t*m=(spindle_motor_t*)arg->p_context;
    timer_info_t i;
    m->speed_timer->p_api->infoGet(m->speed_timer->p_ctrl,&i);
    if(arg->event==TIMER_EVENT_CAPTURE_A){
        const uint32_t capture = arg->capture;
        const uint32_t counter_hz=i.clock_frequency;
        const int rpm=(capture!=0)?(60*counter_hz/capture):0;
        m->current_rpm=rpm;
    }
}

/* spindle entry function */
/* pvParameters contains TaskHandle_t */
void spindle_control_task_entry(void* pvParameters) {
    FSP_PARAMETER_NOT_USED(pvParameters);
    TickType_t last_wake_time = xTaskGetTickCount();
    /* TODO: add your own code here */
    while (1) {
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(10));
        int rpm=g_spindle_motor.current_rpm;
        if(g_spindle_motor.motor_enabled==pdFALSE){
            R_IOPORT_PinWrite(&g_ioport_ctrl, g_spindle_motor.motor_on, BSP_IO_LEVEL_LOW);
            g_spindle_motor.pwm_timer->p_api->dutyCycleSet(g_spindle_motor.pwm_timer->p_ctrl, 0, GPT_IO_PIN_GTIOCA);
            continue;
        }
        R_IOPORT_PinWrite(&g_ioport_ctrl, g_spindle_motor.motor_on, BSP_IO_LEVEL_HIGH);
        int err = rpm - g_spindle_motor.target_rpm;
        g_spindle_motor.err_integral += err;
        int derr = err - g_spindle_motor.prev_err;
        g_spindle_motor.prev_err = err;
        int ctrl = (int)(g_spindle_motor.kp * err + g_spindle_motor.kd * derr + g_spindle_motor.ki * g_spindle_motor.err_integral);
        if (ctrl < 0) {
            ctrl = 0;  // duty is positive.
        }
        g_spindle_motor.pwm_timer->p_api->dutyCycleSet(g_spindle_motor.pwm_timer->p_ctrl, ctrl, GPT_IO_PIN_GTIOCA);
    }
}

void spindle_init(spindle_motor_t* m) {
    m->motor_alarm = SPINDLE_ALERT;
    m->motor_break = SPINDLE_BREAK;
    m->motor_dir = SPINDLE_DIR;
    m->motor_on = SPINDLE_ON;
    m->pwm_timer = &g_timer_spindle_pwm;
    m->speed_timer = &g_timer_spindle_speed;
    m->speed_timer->p_api->open(m->speed_timer->p_ctrl, m->speed_timer->p_cfg);
    m->pwm_timer->p_api->open(m->pwm_timer->p_ctrl, m->pwm_timer->p_cfg);
    m->speed_timer->p_api->callbackSet(m->speed_timer->p_ctrl,tmr_callback,m,NULL);
    m->speed_timer->p_api->enable(m->speed_timer->p_ctrl);
    m->speed_timer->p_api->start(m->speed_timer->p_ctrl);
    m->pwm_timer->p_api->enable(m->pwm_timer->p_ctrl);
    m->pwm_timer->p_api->start(m->pwm_timer->p_ctrl);
    m->motor_enabled=pdFALSE;
    m->target_rpm=0;
}
void spindle_set_speed(spindle_motor_t* m, int rpm) {
    m->target_rpm = rpm;
}
int spindle_get_speed(spindle_motor_t* m) {
    int rpm = m->current_rpm;
    return rpm;
}
void spindle_set_duty(spindle_motor_t* m, uint32_t duty) {
    m->pwm_timer->p_api->dutyCycleSet(m->pwm_timer->p_ctrl, duty, GPT_IO_PIN_GTIOCA);
}
spindle_status_t spindle_get_status(spindle_motor_t* m) {
    bsp_io_level_t level;
    R_IOPORT_PinRead(&g_ioport_ctrl, m->motor_alarm, &level);
    if (level == BSP_IO_LEVEL_LOW) {
        return SPINDLE_ERROR;
    }
    return SPINDLE_OK;
}
void spindle_enable(spindle_motor_t* m, BaseType_t on_off) {
    m->motor_enabled = on_off;
}

void spindle_set_control_param(spindle_motor_t*m,float kp,float ki,float kd){
    m->kp=kp;
    m->ki=ki;
    m->kd=kd;
}