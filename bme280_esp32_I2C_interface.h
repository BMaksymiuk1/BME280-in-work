#ifndef BME280_ESP32_I2C_INTERFACE_H
#define BME280_ESP32_I2C_INTERFACE_H

#include "driver/i2c.h"
#include "bme280_driver.h" 
#include "bme280.h"

typedef struct {
    i2c_port_t I2C_Port_Num; // I2C_NUM_0 or I2C_NUM_1
    uint8_t I2C_Address;     // 0x76 or 0x77
} ESP32_I2C_Config_t;

// Deklaracja publicznej funkcji inicjalizującej sprzęt
void BME280_ESP32_I2C_Interface_Init(BME280_Device_t *Device, ESP32_I2C_Config_t *Config);




#endif // BME280_ESP32_I2C_INTERFACE_H