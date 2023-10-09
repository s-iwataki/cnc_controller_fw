/* generated vector source file - do not edit */
        #include "bsp_api.h"
        /* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
        #if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_MAX_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = adc0_scan_completed, /* ADC0 SCAN END (A/D scan end interrupt) */
            [1] = adc1_scan_completed, /* ADC1 SCAN END (A/D scan end interrupt) */
        };
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_MAX_ENTRIES] =
        {
            [0] = BSP_PRV_IELS_ENUM(EVENT_ADC0_SCAN_END), /* ADC0 SCAN END (A/D scan end interrupt) */
            [1] = BSP_PRV_IELS_ENUM(EVENT_ADC1_SCAN_END), /* ADC1 SCAN END (A/D scan end interrupt) */
        };
        #endif