#include "ra_spi.h"

#include <stddef.h>

#include "FreeRTOS.h"
#include "container_util.h"
#include "hal_data.h"
#include "portmacro.h"
#include "projdefs.h"
#include "r_spi_api.h"
#include "semphr.h"
#include "spi.h"
#include "task.h"

typedef struct {
    spi_bus_driver_t api;
    const spi_instance_t* fsp_inst;
    SemaphoreHandle_t bus_lock;
    StaticSemaphore_t bus_lock_mem;
    TaskHandle_t task_handle;
} ra_spi_busdriver_t;

void spi_callback(spi_callback_args_t* p_arg) {
    ra_spi_busdriver_t* d = container_of(p_arg->p_context, ra_spi_busdriver_t, api);
    if (d->task_handle != NULL) {
        BaseType_t higher_prio_task_wake;
        xTaskNotifyFromISR(d->task_handle, p_arg->event, eSetBits, &higher_prio_task_wake);
        portYIELD_FROM_ISR(higher_prio_task_wake);
    }
}

static int spi_do_transaction(struct spi_bus_driver_s* instance, struct spi_bus_device_s* device, char* tx_buf, size_t tx_size, char* rx_buf, size_t rx_size) {
    ra_spi_busdriver_t* d = container_of(instance, ra_spi_busdriver_t, api);
    const spi_instance_t* inst = d->fsp_inst;
    d->task_handle = xTaskGetCurrentTaskHandle();
    size_t common_txrx_size = (tx_size > rx_size) ? (tx_size - rx_size) : (rx_size - tx_size);
    if (common_txrx_size > 0) {
        inst->p_api->writeRead(inst->p_ctrl, tx_buf, rx_buf, common_txrx_size, SPI_BIT_WIDTH_8_BITS);
        uint32_t notify_value;
        uint32_t spi_event_mask = SPI_EVENT_TRANSFER_COMPLETE;
        while (1) {
            xTaskNotifyWait(spi_event_mask, spi_event_mask, &notify_value, portMAX_DELAY);
            if (notify_value & SPI_EVENT_TRANSFER_COMPLETE) {
                break;
            }
        }
    }
    size_t tx_remain = tx_size - common_txrx_size;
    size_t rx_remain = rx_size - common_txrx_size;
    if (tx_remain > 0) {
        inst->p_api->write(inst->p_ctrl, tx_buf + common_txrx_size, tx_remain, SPI_BIT_WIDTH_8_BITS);
        uint32_t notify_value;
        uint32_t spi_event_mask = SPI_EVENT_TRANSFER_COMPLETE;
        while (1) {
            xTaskNotifyWait(spi_event_mask, spi_event_mask, &notify_value, portMAX_DELAY);
            if (notify_value & SPI_EVENT_TRANSFER_COMPLETE) {
                break;
            }
        }
    }
    if (rx_remain > 0) {
        inst->p_api->read(inst->p_ctrl, rx_buf + common_txrx_size, rx_remain, SPI_BIT_WIDTH_8_BITS);
        uint32_t notify_value;
        uint32_t spi_event_mask = SPI_EVENT_TRANSFER_COMPLETE;
        while (1) {
            xTaskNotifyWait(spi_event_mask, spi_event_mask, &notify_value, portMAX_DELAY);
            if (notify_value & SPI_EVENT_TRANSFER_COMPLETE) {
                break;
            }
        }
    }
    d->task_handle = 0;
    if (device->on_transfer_completed) {
        device->on_transfer_completed(device);
    }
    return 0;
}
static int spi_aquire_lock(struct spi_bus_driver_s* instance, struct spi_bus_device_s* device) {
    ra_spi_busdriver_t* d = container_of(instance, ra_spi_busdriver_t, api);
    if (xSemaphoreTake(d->bus_lock, 0) == pdFAIL) {
        return -1;
    }
    if (device->on_bus_aquired) {
        device->on_bus_aquired(device);
    }
    return 0;
}
static int spi_release_lock(struct spi_bus_driver_s* instance, struct spi_bus_device_s* device) {
    ra_spi_busdriver_t* d = container_of(instance, ra_spi_busdriver_t, api);
    xSemaphoreGive(d->bus_lock);
    if (device->on_bus_released) {
        device->on_bus_released(device);
    }
    return 0;
}

static void disable(struct spi_bus_driver_s* instance, struct spi_bus_device_s* device) {
    ra_spi_busdriver_t* d = container_of(instance, ra_spi_busdriver_t, api);
    const spi_instance_t* inst = d->fsp_inst;
    inst->p_api->close(inst->p_ctrl);
}
static void enable(struct spi_bus_driver_s* instance, struct spi_bus_device_s* device) {
    ra_spi_busdriver_t* d = container_of(instance, ra_spi_busdriver_t, api);
    const spi_instance_t* inst = d->fsp_inst;
    inst->p_api->open(inst->p_ctrl, inst->p_cfg);
}

static ra_spi_busdriver_t driver;

spi_bus_driver_t* spi_init(void) {
    driver.api.disable = disable;
    driver.api.enable = enable;
    driver.api.spi_aquire_lock = spi_aquire_lock;
    driver.api.spi_do_transaction = spi_do_transaction;
    driver.api.spi_release_lock = spi_release_lock;
    driver.fsp_inst = &g_spi0;
    g_spi0.p_api->open(g_spi0.p_ctrl, g_spi0.p_cfg);
    g_spi0.p_api->callbackSet(g_spi0.p_ctrl, spi_callback, &driver, NULL);
    driver.bus_lock = xSemaphoreCreateMutexStatic(&driver.bus_lock_mem);
    driver.task_handle = 0;
    return &(driver.api);
}