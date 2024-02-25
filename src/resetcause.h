#pragma once

typedef enum{
    RESET_POWERON,
    RESET_LOWVOLTAGE,
    RESET_DEEPSLEEP,
    RESET_IWDT,
    RESET_WDT,
    RESET_SOFTWARE,
    RESET_SRAM_PERR,
    RESET_BUS_SLAVE_ERR,
    RESET_BUS_MASTER_ERROR,
    RESET_SP_ERROR,
    RESET_EXTERNAL,
    RESET_UNKNOWN
}reset_cause_t ;


reset_cause_t reset_cause_get(void);
const char* reset_cause_get_str(void);
void reset_cause_checked(void);