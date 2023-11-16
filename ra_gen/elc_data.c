/* generated ELC source file - do not edit */
        #include "r_elc_api.h"
        const elc_cfg_t g_elc_cfg = {
                        .link[ELC_PERIPHERAL_GPT_A] = ELC_EVENT_ELC_SOFTWARE_EVENT_0, /* ELC SOFTWARE EVENT 0 (Software event 0) */
            .link[ELC_PERIPHERAL_GPT_B] = ELC_EVENT_GPT0_CAPTURE_COMPARE_A, /* GPT0 CAPTURE COMPARE A (Compare match A) */
            .link[ELC_PERIPHERAL_GPT_C] = ELC_EVENT_GPT2_CAPTURE_COMPARE_A, /* GPT2 CAPTURE COMPARE A (Compare match A) */
            .link[ELC_PERIPHERAL_GPT_D] = ELC_EVENT_GPT5_CAPTURE_COMPARE_A, /* GPT5 CAPTURE COMPARE A (Compare match A) */
            .link[ELC_PERIPHERAL_GPT_E] = ELC_EVENT_GPT1_COUNTER_OVERFLOW, /* GPT1 COUNTER OVERFLOW (Overflow) */
            .link[ELC_PERIPHERAL_GPT_F] = ELC_EVENT_GPT8_COUNTER_OVERFLOW, /* GPT8 COUNTER OVERFLOW (Overflow) */
            .link[ELC_PERIPHERAL_GPT_G] = ELC_EVENT_GPT3_COUNTER_OVERFLOW, /* GPT3 COUNTER OVERFLOW (Overflow) */
            .link[ELC_PERIPHERAL_GPT_H] = ELC_EVENT_NONE, /* No allocation */
            .link[ELC_PERIPHERAL_ADC0] = ELC_EVENT_NONE, /* No allocation */
            .link[ELC_PERIPHERAL_ADC0_B] = ELC_EVENT_NONE, /* No allocation */
            .link[ELC_PERIPHERAL_ADC1] = ELC_EVENT_NONE, /* No allocation */
            .link[ELC_PERIPHERAL_ADC1_B] = ELC_EVENT_NONE, /* No allocation */
            .link[ELC_PERIPHERAL_DAC0] = ELC_EVENT_NONE, /* No allocation */
            .link[ELC_PERIPHERAL_DAC1] = ELC_EVENT_NONE, /* No allocation */
            .link[ELC_PERIPHERAL_IOPORT1] = ELC_EVENT_NONE, /* No allocation */
            .link[ELC_PERIPHERAL_IOPORT2] = ELC_EVENT_NONE, /* No allocation */
            .link[ELC_PERIPHERAL_IOPORT3] = ELC_EVENT_NONE, /* No allocation */
            .link[ELC_PERIPHERAL_IOPORT4] = ELC_EVENT_NONE, /* No allocation */
        };

#if BSP_TZ_SECURE_BUILD

        void R_BSP_ElcCfgSecurityInit(void);

        /* Configure ELC Security Attribution. */
        void R_BSP_ElcCfgSecurityInit(void)
        {
 #if (2U == BSP_FEATURE_ELC_VERSION)
            uint32_t elcsarbc = UINT32_MAX;

            elcsarbc &=  ~(1U << ELC_PERIPHERAL_GPT_A);
            elcsarbc &=  ~(1U << ELC_PERIPHERAL_GPT_B);
            elcsarbc &=  ~(1U << ELC_PERIPHERAL_GPT_C);
            elcsarbc &=  ~(1U << ELC_PERIPHERAL_GPT_D);
            elcsarbc &=  ~(1U << ELC_PERIPHERAL_GPT_E);
            elcsarbc &=  ~(1U << ELC_PERIPHERAL_GPT_F);
            elcsarbc &=  ~(1U << ELC_PERIPHERAL_GPT_G);

            /* Write the settings to ELCSARn Registers. */
            R_ELC->ELCSARA = 0xFFFFFFFEU;
            R_ELC->ELCSARB = elcsarbc;
 #else
            uint16_t elcsarbc[2] = {0xFFFFU, 0xFFFFU};
            elcsarbc[ELC_PERIPHERAL_GPT_A / 16U] &= (uint16_t) ~(1U << (ELC_PERIPHERAL_GPT_A % 16U));
            elcsarbc[ELC_PERIPHERAL_GPT_B / 16U] &= (uint16_t) ~(1U << (ELC_PERIPHERAL_GPT_B % 16U));
            elcsarbc[ELC_PERIPHERAL_GPT_C / 16U] &= (uint16_t) ~(1U << (ELC_PERIPHERAL_GPT_C % 16U));
            elcsarbc[ELC_PERIPHERAL_GPT_D / 16U] &= (uint16_t) ~(1U << (ELC_PERIPHERAL_GPT_D % 16U));
            elcsarbc[ELC_PERIPHERAL_GPT_E / 16U] &= (uint16_t) ~(1U << (ELC_PERIPHERAL_GPT_E % 16U));
            elcsarbc[ELC_PERIPHERAL_GPT_F / 16U] &= (uint16_t) ~(1U << (ELC_PERIPHERAL_GPT_F % 16U));
            elcsarbc[ELC_PERIPHERAL_GPT_G / 16U] &= (uint16_t) ~(1U << (ELC_PERIPHERAL_GPT_G % 16U));

            /* Write the settins to ELCSARn Registers. */
            R_ELC->ELCSARA = 0xFFFEU;
            R_ELC->ELCSARB = elcsarbc[0];
            R_ELC->ELCSARC = elcsarbc[1];
 #endif
        }
#endif