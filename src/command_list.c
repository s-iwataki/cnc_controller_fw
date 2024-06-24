#include "command_list.h"
#include "commands/test_cmd.h"
#include "commands/gcode/gcode_cmd.h"
#include <stdint.h>
#include <stddef.h>
command_entry_t command_list[]={
    {"test",test_cmd,"test the equipment's peripherals."},
    {"gcode",gcode_cmd,"enter gcode mode."},
    {NULL,NULL,NULL}
};