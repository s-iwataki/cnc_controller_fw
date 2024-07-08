#include "cnc_systemstate.h"
static cnc_system_state_t state;

void cnc_system_state_init(){
    state.is_g_code_exec=0;
    state.g_code_state.abs_incr=G91_INCR;
    state.g_code_state.feed=60.0f;//60mm/min=1mm/sec
    state.g_code_state.interpolation_mode=G00_FAST_FEED;
    state.g_code_state.spindle_rpm_ref=2000.0f;
    state.g_code_state.working_plane=G17_XY_PLANE;
}
int cnc_system_state_is_gcode_exec(){
    return 0;
}

g_code_state_t* cnc_system_state_set_gcode_exec(){
    state.is_g_code_exec=1;
    return &state.g_code_state;
}
void cnc_system_state_unset_gcode_exec(){
    state.is_g_code_exec=0;
}