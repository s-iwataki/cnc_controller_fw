#pragma once
#include "FreeRTOS.h"
#define MAIN_TASK_STACK_DEPTH 512
extern StackType_t main_task_stack[MAIN_TASK_STACK_DEPTH];
extern StaticTask_t main_task_tcb;
void task_main(void*param);