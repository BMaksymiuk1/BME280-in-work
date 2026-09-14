//////////////////////////////////////////////    BME280 I2C INTERFACE FOR ESP32   ///////////////////////////////////////////////////////

// User is responsible for creating the I2C bus
// This API only initializes a device on an existing bus


#include "bme280_esp32_I2C_interface.h"

#include "driver/i2c_master.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stddef.h>
#include <stdint.h>

#define I2C_TIMEOUT_MS 1000

esp_err_t status;


static int8_t ESP32_I2C_ReadReg(
    void *InterfacePtr,
    uint8_t RegisterAddress,
    uint8_t *Buffer,
    uint8_t Length)
{
    if (InterfacePtr == NULL || Buffer == NULL)
    {
        return I2C_ESP32_NULL_PTR;
    }

    I2C_ESP32_Config_t *Config =
        (I2C_ESP32_Config_t *)InterfacePtr;

    if (Config->I2C_ESP32_DeviceHandle == NULL)
    {
        return I2C_ESP32_ERROR;
    }

    esp_err_t err = i2c_master_transmit_receive(
        Config->I2C_ESP32_DeviceHandle,
        &RegisterAddress,
        1,
        Buffer,
        Length,
        I2C_TIMEOUT_MS
    );

    if (err != ESP_OK)
    {
        printf(
            "I2C READ ERROR: reg=0x%02X len=%u err=%s (0x%X)\n",
            RegisterAddress,
            Length,
            esp_err_to_name(err),
            (unsigned)err
        );

        return I2C_ESP32_ERROR;
    }

    return I2C_ESP32_OK;
}

static int8_t ESP32_I2C_WriteReg(
    void *InterfacePtr,
    uint8_t RegisterAddress,
    uint8_t NewByte)
{
    if (InterfacePtr == NULL)
    {
        return I2C_ESP32_NULL_PTR;
    }

    I2C_ESP32_Config_t *Config =
        (I2C_ESP32_Config_t *)InterfacePtr;

    if (Config->I2C_ESP32_DeviceHandle == NULL)
    {
        return I2C_ESP32_ERROR;
    }

    uint8_t WriteBuffer[2] =
    {
        RegisterAddress,
        NewByte
    };

    esp_err_t err = i2c_master_transmit(
        Config->I2C_ESP32_DeviceHandle,
        WriteBuffer,
        sizeof(WriteBuffer),
        I2C_TIMEOUT_MS
    );

    if (err != ESP_OK)
    {
        printf(
            "I2C WRITE ERROR: reg=0x%02X data=0x%02X err=%s (0x%X)\n",
            RegisterAddress,
            NewByte,
            esp_err_to_name(err),
            (unsigned)err
        );

        return I2C_ESP32_ERROR;
    }

    return I2C_ESP32_OK;
}

static void ESP32_DelayMs(uint16_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

I2C_ESP32_Error_t BME280_ESP32_I2C_Interface_Init(BME280_Device_t *Device, I2C_ESP32_Config_t *Config)
{
    if(Device == NULL || Config == NULL)
    {
        return I2C_ESP32_NULL_PTR;
    }

    if(Config->I2C_ESP32_BusHandle == NULL)
    {
        return I2C_ESP32_ERROR;
    }

    i2c_device_config_t DeviceConfig = // structure describing device to add to a bus
    { 
    .dev_addr_length = I2C_ADDR_BIT_LEN_7, //Device's address lenght (for bme280 its 7 - 7 adress bits + 1 read/write bit)
    .device_address = Config->I2C_ESP32_Address, //For bme280 0x76 or 0x77
    .scl_speed_hz = Config->I2C_ESP32_SpeedHz //BME280 supports 3.4MHz I2C communication, ESP-IDF says it shoult NOT be larger than 400KHz
    };

    //Creating new device on bus, writing device handle to &Config->DeviceHandle
    esp_err_t err = i2c_master_bus_add_device(Config->I2C_ESP32_BusHandle, &DeviceConfig, &Config->I2C_ESP32_DeviceHandle);
    if (err != ESP_OK)
    {
        Config->I2C_ESP32_DeviceHandle = NULL;
        return I2C_ESP32_ERROR;
    }

    Device->InterfacePtr = Config;                  //Connecting interface to driver
    Device->driver.ReadReg = ESP32_I2C_ReadReg;
    Device->driver.WriteReg = ESP32_I2C_WriteReg;
    Device->driver.DelayMs = ESP32_DelayMs;
    return I2C_ESP32_OK;
}

