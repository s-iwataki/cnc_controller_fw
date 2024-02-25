#include <stdio.h>
#include <string.h>

#include "command_list.h"
#include "resetcause.h"
#include "shell.h"
#include "syscalls.h"

/* shell task entry function */
/* pvParameters contains TaskHandle_t */
char cmd_buff[256];
char* argv[16];

static void print_available_cmd(void) {
    printf("available commands:\r\n");
    for (int i = 0; command_list[i].cmd_name != NULL; i++) {
        printf("%s: %s\r\n", command_list[i].cmd_name, command_list[i].help_text);
    }
}
void shell_entry(void* pvParameters) {
    FSP_PARAMETER_NOT_USED(pvParameters);
    printf("cnc controller\r\n");
    printf("reset: %s\r\n", reset_cause_get_str());
    reset_cause_t reset_cause = reset_cause_get();
    if (!(reset_cause == RESET_EXTERNAL) || (reset_cause == RESET_POWERON)) {
        syscall_print_debug_dump();
    }
    reset_cause_checked();
    /* TODO: add your own code here */
    while (1) {
        printf(">");
        fgets(cmd_buff, sizeof(cmd_buff), stdin);
        char* token = strtok(cmd_buff, " \n\t");
        if (token == NULL) {
            continue;
        }
        int argc = 0;
        for (; (argc < sizeof(argv)) && (token != NULL); token = strtok(NULL, " \n\t"), argc++) {
            argv[argc] = token;
        }
        int cmd_found = 0;
        for (int i = 0; command_list[i].cmd_name != NULL; i++) {
            if (strcmp(command_list[i].cmd_name, argv[0]) == 0) {
                int retval = command_list[i].cmd_func(argc, argv);
                if (retval != 0) {
                    printf("command_fail:%d\r\n", retval);
                }
                cmd_found = 1;
                break;
            }
        }
        if (cmd_found) {
            continue;
        }
        print_available_cmd();
    }
}
