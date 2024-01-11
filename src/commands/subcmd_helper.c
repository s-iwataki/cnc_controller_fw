#include "subcmd_helper.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>

void print_available_subcmd(const command_entry_t* cmds) {
    printf("available sub commands:\r\n");
    for (int i = 0; cmds[i].cmd_name != NULL; i++) {
        printf("%s: %s\r\n", cmds[i].cmd_name, cmds[i].help_text);
    }
}

int apply_sub_cmd(int argc, char** argv, const command_entry_t* cmds) {
    if (argc < 2) {
        print_available_subcmd(cmds);
        return 0;
    }
    for (int i = 0; cmds[i].cmd_name != NULL; i++) {
        if (strcmp(argv[1], cmds[i].cmd_name) == 0) {
            return cmds[i].cmd_func(argc - 1, &argv[1]);
        }
    }
    printf("unknown subcommand:%s\r\n", argv[1]);
    print_available_subcmd(cmds);
    return 0;
}