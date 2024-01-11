#pragma once
#include "command_list.h"

void print_available_subcmd(const command_entry_t* cmds);
int apply_sub_cmd(int argc, char** argv, const command_entry_t* cmds);