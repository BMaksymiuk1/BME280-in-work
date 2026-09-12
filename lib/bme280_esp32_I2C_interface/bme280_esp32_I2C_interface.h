#ifndef BME280_ESP32_I2C_INTERFACE_H
#define BME280_ESP32_I2C_INTERFACE_H

#include "bme280.h"
#include "bme280_driver.h"

#include "driver/i2c_master.h"

#include <stdint.h>

typedef enum {
    I2C_ESP32_OK = 0,
    I2C_ESP32_NULL_PTR = -1,
    I2C_ESP32_ERROR = -2
} I2C_ESP32_Error_t;

typedef struct
{
    i2c_port_num_t I2C_ESP32_PortNum;

    gpio_num_t I2C_ESP32_SDA_IO;
    gpio_num_t I2C_ESP32_SCL_IO;

    uint8_t I2C_ESP32_Address;
    uint32_t I2C_ESP32_SpeedHz;

    i2c_master_bus_handle_t I2C_ESP32_BusHandle;
    i2c_master_dev_handle_t I2C_ESP32_DeviceHandle;

} I2C_ESP32_Config_t;

#endif // BME280_ESP32_I2C_INTERFACE_H