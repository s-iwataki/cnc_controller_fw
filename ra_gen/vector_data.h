/* generated vector header file - do not edit */
        #ifndef VECTOR_DATA_H
        #define VECTOR_DATA_H
        #ifdef __cplusplus
        extern "C" {
        #endif
                /* Number of interrupts allocated */
        #ifndef VECTOR_DATA_IRQ_COUNT
        #define VECTOR_DATA_IRQ_COUNT    (2)
        #endif
        /* ISR prototypes */
        void adc0_scan_completed(void);
        void adc1_scan_completed(void);

        /* Vector table allocations */
        #define VECTOR_NUMBER_ADC0_SCAN_END ((IRQn_Type) 0) /* ADC0 SCAN END (A/D scan end interrupt) */
        #define ADC0_SCAN_END_IRQn          ((IRQn_Type) 0) /* ADC0 SCAN END (A/D scan end interrupt) */
        #define VECTOR_NUMBER_ADC1_SCAN_END ((IRQn_Type) 1) /* ADC1 SCAN END (A/D scan end interrupt) */
        #define ADC1_SCAN_END_IRQn          ((IRQn_Type) 1) /* ADC1 SCAN END (A/D scan end interrupt) */
        #ifdef __cplusplus
        }
        #endif
        #endif /* VECTOR_DATA_H */