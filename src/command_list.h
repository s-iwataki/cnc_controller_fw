#pragma once

typedef struct{
    const char*const cmd_name;
    int (*cmd_func)(int argc, char**argv);
    const char*const help_text;
}command_entry_t;

extern command_entry_t command_list[];