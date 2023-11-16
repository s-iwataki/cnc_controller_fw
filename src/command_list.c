#include "command_list.h"
#include "commands/test_cmd.h"
#include <stdint.h>
#include <stddef.h>
command_entry_t command_list[]={
    {"test",test_cmd,"test the equipment's peripherals."},
    {NULL,NULL,NULL}
};