/* generated vector header file - do not edit */
        #ifndef VECTOR_DATA_H
        #define VECTOR_DATA_H
        #ifdef __cplusplus
        extern "C" {
        #endif
                /* Number of interrupts allocated */
        #ifndef VECTOR_DATA_IRQ_COUNT
        #define VECTOR_DATA_IRQ_COUNT    (16)
        #endif
        /* ISR prototypes */
        void sci_uart_rxi_isr(void);
        void sci_uart_txi_isr(void);
        void sci_uart_tei_isr(void);
        void sci_uart_eri_isr(void);
        void gpt_capture_a_isr(void);
        void spi_tei_isr(void);
        void spi_eri_isr(void);
        void dmac_int_isr(void);
        void adc_scan_end_isr(void);
        void fcu_frdyi_isr(void);
        void fcu_fiferr_isr(void);

        /* Vector table allocations */
        #define VECTOR_NUMBER_SCI9_RXI ((IRQn_Type) 0) /* SCI9 RXI (Received data full) */
        #define SCI9_RXI_IRQn          ((IRQn_Type) 0) /* SCI9 RXI (Received data full) */
        #define VECTOR_NUMBER_SCI9_TXI ((IRQn_Type) 1) /* SCI9 TXI (Transmit data empty) */
        #define SCI9_TXI_IRQn          ((IRQn_Type) 1) /* SCI9 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI9_TEI ((IRQn_Type) 2) /* SCI9 TEI (Transmit end) */
        #define SCI9_TEI_IRQn          ((IRQn_Type) 2) /* SCI9 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI9_ERI ((IRQn_Type) 3) /* SCI9 ERI (Receive error) */
        #define SCI9_ERI_IRQn          ((IRQn_Type) 3) /* SCI9 ERI (Receive error) */
        #define VECTOR_NUMBER_GPT0_CAPTURE_COMPARE_A ((IRQn_Type) 4) /* GPT0 CAPTURE COMPARE A (Compare match A) */
        #define GPT0_CAPTURE_COMPARE_A_IRQn          ((IRQn_Type) 4) /* GPT0 CAPTURE COMPARE A (Compare match A) */
        #define VECTOR_NUMBER_GPT2_CAPTURE_COMPARE_A ((IRQn_Type) 5) /* GPT2 CAPTURE COMPARE A (Compare match A) */
        #define GPT2_CAPTURE_COMPARE_A_IRQn          ((IRQn_Type) 5) /* GPT2 CAPTURE COMPARE A (Compare match A) */
        #define VECTOR_NUMBER_GPT5_CAPTURE_COMPARE_A ((IRQn_Type) 6) /* GPT5 CAPTURE COMPARE A (Compare match A) */
        #define GPT5_CAPTURE_COMPARE_A_IRQn          ((IRQn_Type) 6) /* GPT5 CAPTURE COMPARE A (Compare match A) */
        #define VECTOR_NUMBER_GPT7_CAPTURE_COMPARE_A ((IRQn_Type) 7) /* GPT7 CAPTURE COMPARE A (Compare match A) */
        #define GPT7_CAPTURE_COMPARE_A_IRQn          ((IRQn_Type) 7) /* GPT7 CAPTURE COMPARE A (Compare match A) */
        #define VECTOR_NUMBER_SPI0_TEI ((IRQn_Type) 8) /* SPI0 TEI (Transmission complete event) */
        #define SPI0_TEI_IRQn          ((IRQn_Type) 8) /* SPI0 TEI (Transmission complete event) */
        #define VECTOR_NUMBER_SPI0_ERI ((IRQn_Type) 9) /* SPI0 ERI (Error) */
        #define SPI0_ERI_IRQn          ((IRQn_Type) 9) /* SPI0 ERI (Error) */
        #define VECTOR_NUMBER_DMAC0_INT ((IRQn_Type) 10) /* DMAC0 INT (DMAC transfer end 0) */
        #define DMAC0_INT_IRQn          ((IRQn_Type) 10) /* DMAC0 INT (DMAC transfer end 0) */
        #define VECTOR_NUMBER_DMAC1_INT ((IRQn_Type) 11) /* DMAC1 INT (DMAC transfer end 1) */
        #define DMAC1_INT_IRQn          ((IRQn_Type) 11) /* DMAC1 INT (DMAC transfer end 1) */
        #define VECTOR_NUMBER_ADC0_SCAN_END ((IRQn_Type) 12) /* ADC0 SCAN END (A/D scan end interrupt) */
        #define ADC0_SCAN_END_IRQn          ((IRQn_Type) 12) /* ADC0 SCAN END (A/D scan end interrupt) */
        #define VECTOR_NUMBER_ADC1_SCAN_END ((IRQn_Type) 13) /* ADC1 SCAN END (A/D scan end interrupt) */
        #define ADC1_SCAN_END_IRQn          ((IRQn_Type) 13) /* ADC1 SCAN END (A/D scan end interrupt) */
        #define VECTOR_NUMBER_FCU_FRDYI ((IRQn_Type) 14) /* FCU FRDYI (Flash ready interrupt) */
        #define FCU_FRDYI_IRQn          ((IRQn_Type) 14) /* FCU FRDYI (Flash ready interrupt) */
        #define VECTOR_NUMBER_FCU_FIFERR ((IRQn_Type) 15) /* FCU FIFERR (Flash access error interrupt) */
        #define FCU_FIFERR_IRQn          ((IRQn_Type) 15) /* FCU FIFERR (Flash access error interrupt) */
        #ifdef __cplusplus
        }
        #endif
        #endif /* VECTOR_DATA_H */