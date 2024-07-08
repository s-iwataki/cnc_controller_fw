#pragma once
#include <stdint.h>
#include "cnc_systemstate.h"

#define MAX_FEEDSPEED_MM_PER_SEC 40
#define MAX_SPINDLE_RPM 5000


typedef enum{
    G90_ABS,
    G91_INCR,
    G90_91_UNSET
}g_code_abs_inc_state_t;

typedef enum{
    G00_FAST_FEED,
    G01_LINEAR,
    G02_CIRCULAR_CW,
    G03_CIRCULAR_CCW,
    G01_03_UNSET
}g_code_interpolation_state_t;

typedef enum{
    G17_XY_PLANE,
    G18_ZX_PLANE,
    G19_YZ_PLANE,
    G17_19_UNSET
}g_code_working_plane_t;
typedef enum{
    M03_MOTOR_ON,
    M05_MOTOR_OFF,
    M03_05_UNSET
}g_code_motor_cmd_t;

typedef struct{
    g_code_abs_inc_state_t abs_incr;
    g_code_interpolation_state_t interpolation_mode;
    g_code_working_plane_t working_plane;
    float feed;//mm/min
    float spindle_rpm_ref;//rpm
}g_code_state_t;

typedef struct{
    g_code_state_t g_code_state;
    int is_g_code_exec;

}cnc_system_state_t;

void cnc_system_state_init();
int cnc_system_state_is_gcode_exec();
g_code_state_t* cnc_system_state_set_gcode_exec();
void cnc_system_state_unset_gcode_exec();