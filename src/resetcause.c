#include "resetcause.h"

#include "bsp_api.h"

reset_cause_t reset_cause_get(void) {
    uint16_t rstsr0 = R_SYSTEM->RSTSR0;
    if (rstsr0 & 0x0e) {
        return RESET_LOWVOLTAGE;
    }
    if (rstsr0 & 0x80) {
        return RESET_DEEPSLEEP;
    }
    uint16_t rstsr1 = R_SYSTEM->RSTSR1;
    if (rstsr1 & 0x01) {
        return RESET_IWDT;
    }
    if (rstsr1 & 0x02) {
        return RESET_WDT;
    }
    if (rstsr1 & 0x04) {
        return RESET_SOFTWARE;
    }
    if (rstsr1 & (1 << 10)) {
        return RESET_BUS_SLAVE_ERR;
    }
    if (rstsr1 & (1 << 11)) {
        return RESET_BUS_MASTER_ERROR;
    }
    if (rstsr1 & (1 << 12)) {
        return RESET_SP_ERROR;
    }
    if (rstsr0 & 0x01) {
        return RESET_POWERON;
    }
    if (R_SYSTEM->RSTSR2 & 0x01) {
        return RESET_EXTERNAL;
    }
    return RESET_UNKNOWN;
}

static const char* reset_description[] = {
    "Power on Reset",
    "Low voltage Reset",
    "Deep sleep Reset",
    "Independent WDT Reset",
    "WDT Reset",
    "Software Reset",
    "SRAM parity error Reset",
    "Bus slave MPU error Reset",
    "Bus master MPU error Reset",
    "Stack pointer error Reset",
    "External Reset",
    "Unknown Reset"};

const char* reset_cause_get_str(void) {
    reset_cause_t cause = reset_cause_get();
    if (cause > RESET_UNKNOWN) {
        cause = RESET_UNKNOWN;
    }
    return reset_description[cause];
}
void reset_cause_checked(void) {
    R_SYSTEM->RSTSR0 = 0;
    R_SYSTEM->RSTSR1 = 0;
    R_SYSTEM->RSTSR2 = 1;
}