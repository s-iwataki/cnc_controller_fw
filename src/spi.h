#pragma once

#include <stdint.h>
#include <stddef.h>

struct spi_bus_driver_s;
struct spi_bus_device_s;


typedef enum{
    SPI_MODE0,
    SPI_MODE1,
    SPI_MODE2,
    SPI_MODE3
}SPI_MODE_t;

typedef enum{
    SPI_MSB_FISRT,
    SPI_LSB_FIRST
}SPI_BYTE_ORDER_t;

typedef struct{
    SPI_MODE_t mode;
    SPI_BYTE_ORDER_t byte_order;
}spi_config_t;

typedef int(*spi_do_transaction_fn)(struct spi_bus_driver_s*instance,struct spi_bus_device_s*device,char*tx_buf,size_t tx_size,char*rx_buf,size_t rx_size);
typedef int (*spi_aquire_lock_fn)(struct spi_bus_driver_s*instance,struct spi_bus_device_s*device);
typedef int (*spi_release_lock_fn)(struct spi_bus_driver_s*instance,struct spi_bus_device_s*device);

typedef struct spi_bus_driver_s{
    spi_aquire_lock_fn spi_aquire_lock;
    spi_release_lock_fn spi_release_lock;
    spi_do_transaction_fn spi_do_transaction;
    void(*disable)(struct spi_bus_driver_s*instance,struct spi_bus_device_s*device);
    void(*enable)(struct spi_bus_driver_s*instance,struct spi_bus_device_s*device);
}spi_bus_driver_t;



typedef struct spi_bus_device_s{
    void(*on_bus_aquired)(struct spi_bus_device_s*);
    void(*on_bus_released)(struct spi_bus_device_s*);
    void(*on_transfer_completed)(struct spi_bus_device_s*);
    spi_config_t config;
}spi_bus_device_t;