/* generated vector source file - do not edit */
        #include "bsp_api.h"
        /* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
        #if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_MAX_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = sci_uart_rxi_isr, /* SCI9 RXI (Received data full) */
            [1] = sci_uart_txi_isr, /* SCI9 TXI (Transmit data empty) */
            [2] = sci_uart_tei_isr, /* SCI9 TEI (Transmit end) */
            [3] = sci_uart_eri_isr, /* SCI9 ERI (Receive error) */
            [4] = gpt_capture_a_isr, /* GPT0 CAPTURE COMPARE A (Compare match A) */
            [5] = gpt_capture_a_isr, /* GPT2 CAPTURE COMPARE A (Compare match A) */
            [6] = gpt_capture_a_isr, /* GPT5 CAPTURE COMPARE A (Compare match A) */
            [7] = gpt_capture_a_isr, /* GPT7 CAPTURE COMPARE A (Compare match A) */
            [8] = spi_tei_isr, /* SPI0 TEI (Transmission complete event) */
            [9] = spi_eri_isr, /* SPI0 ERI (Error) */
            [10] = dmac_int_isr, /* DMAC0 INT (DMAC transfer end 0) */
            [11] = dmac_int_isr, /* DMAC1 INT (DMAC transfer end 1) */
            [12] = adc_scan_end_isr, /* ADC0 SCAN END (A/D scan end interrupt) */
            [13] = adc_scan_end_isr, /* ADC1 SCAN END (A/D scan end interrupt) */
        };
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_MAX_ENTRIES] =
        {
            [0] = BSP_PRV_IELS_ENUM(EVENT_SCI9_RXI), /* SCI9 RXI (Received data full) */
            [1] = BSP_PRV_IELS_ENUM(EVENT_SCI9_TXI), /* SCI9 TXI (Transmit data empty) */
            [2] = BSP_PRV_IELS_ENUM(EVENT_SCI9_TEI), /* SCI9 TEI (Transmit end) */
            [3] = BSP_PRV_IELS_ENUM(EVENT_SCI9_ERI), /* SCI9 ERI (Receive error) */
            [4] = BSP_PRV_IELS_ENUM(EVENT_GPT0_CAPTURE_COMPARE_A), /* GPT0 CAPTURE COMPARE A (Compare match A) */
            [5] = BSP_PRV_IELS_ENUM(EVENT_GPT2_CAPTURE_COMPARE_A), /* GPT2 CAPTURE COMPARE A (Compare match A) */
            [6] = BSP_PRV_IELS_ENUM(EVENT_GPT5_CAPTURE_COMPARE_A), /* GPT5 CAPTURE COMPARE A (Compare match A) */
            [7] = BSP_PRV_IELS_ENUM(EVENT_GPT7_CAPTURE_COMPARE_A), /* GPT7 CAPTURE COMPARE A (Compare match A) */
            [8] = BSP_PRV_IELS_ENUM(EVENT_SPI0_TEI), /* SPI0 TEI (Transmission complete event) */
            [9] = BSP_PRV_IELS_ENUM(EVENT_SPI0_ERI), /* SPI0 ERI (Error) */
            [10] = BSP_PRV_IELS_ENUM(EVENT_DMAC0_INT), /* DMAC0 INT (DMAC transfer end 0) */
            [11] = BSP_PRV_IELS_ENUM(EVENT_DMAC1_INT), /* DMAC1 INT (DMAC transfer end 1) */
            [12] = BSP_PRV_IELS_ENUM(EVENT_ADC0_SCAN_END), /* ADC0 SCAN END (A/D scan end interrupt) */
            [13] = BSP_PRV_IELS_ENUM(EVENT_ADC1_SCAN_END), /* ADC1 SCAN END (A/D scan end interrupt) */
        };
        #endif