#pragma once
#include <stdint.h>
#include "FreeRTOS.h"
#include "fsp_common_api.h"
#include "hal_data.h"
#include "portmacro.h"
#include "projdefs.h"
#include "r_ioport.h"
#include "semphr.h"

typedef enum{
    SPINDLE_DIRECT_DUTY_CONTROL,
    SPINDLE_SPEED_CONTROL
}spindle_control_mode_t;

typedef struct{
    int init;
    bsp_io_port_pin_t motor_alarm;
    bsp_io_port_pin_t motor_break;
    bsp_io_port_pin_t motor_on;
    bsp_io_port_pin_t motor_dir;
    const timer_instance_t*speed_timer;
    const timer_instance_t*pwm_timer;
    int target_rpm;
    int current_rpm;
    int motor_enabled;
    float kp;
    float ki;
    float kd;
    int prev_err;
    int err_integral;
    spindle_control_mode_t control_mode;
}spindle_motor_t ;
typedef enum{
    SPINDLE_OK,
    SPINDLE_ERROR
}spindle_status_t ;

extern spindle_motor_t g_spindle_motor;

void spindle_init(spindle_motor_t*m);
void spindle_set_speed(spindle_motor_t*m,int rpm);
int spindle_get_speed(spindle_motor_t*m);
void spindle_set_duty(spindle_motor_t*m,uint32_t duty);
spindle_status_t spindle_get_status(spindle_motor_t*m);
void spindle_set_control_param(spindle_motor_t*m,float kp,float ki,float kd);
void spindle_enable(spindle_motor_t*m,BaseType_t on_off);
void spindle_control_mode_set(spindle_motor_t*m,spindle_control_mode_t);