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
    uint8_t I2C_ESP32_Address;
    uint32_t I2C_ESP32_SpeedHz;

    i2c_master_bus_handle_t I2C_ESP32_BusHandle;
    i2c_master_dev_handle_t I2C_ESP32_DeviceHandle;

} I2C_ESP32_Config_t;

I2C_ESP32_Error_t BME280_ESP32_I2C_Interface_Init(BME280_Device_t *Device, I2C_ESP32_Config_t *Config);


#endif // BME280_ESP32_I2C_INTERFACE_H