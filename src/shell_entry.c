#include "shell.h"
#include <stdio.h>
/* shell task entry function */
/* pvParameters contains TaskHandle_t */
char cmd_buff[256];

void shell_entry(void* pvParameters) {
    FSP_PARAMETER_NOT_USED(pvParameters);
    printf("cnc controller\r\n");
    /* TODO: add your own code here */
    while (1) {
        printf(">");
        fgets(cmd_buff,sizeof(cmd_buff),stdin);
        printf("input:%s\r\n",cmd_buff);
    }
}
