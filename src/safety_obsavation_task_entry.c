#include "safety_obsavation_task.h"
                /* safety_obsavation entry function */
                /* pvParameters contains TaskHandle_t */
                void safety_obsavation_task_entry(void * pvParameters)
                {
                    FSP_PARAMETER_NOT_USED(pvParameters);

                    /* TODO: add your own code here */
                    while(1)
                    {
                        vTaskDelay(1);
                    }
                }
