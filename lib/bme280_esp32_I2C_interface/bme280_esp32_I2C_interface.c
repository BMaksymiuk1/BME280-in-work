#include "bme280_esp32_I2C_interface.h"

#include "driver/i2c_master.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stddef.h>
#include <stdint.h>

#define I2C_TIMEOUT_MS 1000

esp_err_t status;


static int8_t ESP32_I2C_ReadReg(void *InterfacePtr, uint8_t RegisterAddress, uint8_t *Buffer, uint8_t Length)
{
    if (InterfacePtr == NULL || Buffer == NULL)
    {
        return I2C_ESP32_NULL_PTR;
    }
    I2C_ESP32_Config_t *Config = (I2C_ESP32_Config_t *)InterfacePtr;
    if (Config->I2C_ESP32_DeviceHandle == NULL)
    {
        return I2C_ESP32_ERROR;
    }
    esp_err_t err = i2c_master_transmit_receive(Config->I2C_ESP32_DeviceHandle, &RegisterAddress, 1, Buffer, Length, I2C_TIMEOUT_MS);
    if (err != ESP_OK)
    {
        return I2C_ESP32_ERROR;
    }
    return I2C_ESP32_OK;
}

static ESP32_I2C_WriteReg(void *InterfacePtr,uint8_t RegisterAddress, uint8_t NewByte)
{

}

I2C_ESP32_Error_t BME280_ESP32_I2C_Interface_Init(BME280_Device_t *Device, I2C_ESP32_Config_t *Config)
{
    if(Device != NULL || Config != NULL)
    {
        return I2C_ESP32_NULL_PTR;
    }
    i2c_master_bus_config_t BusConfig =       //Bus config struct
    {
    .i2c_port = Config->I2C_ESP32_PortNum, //Port used by I2C controler
    .sda_io_num = Config->I2C_ESP32_SDA_IO, //GPIO pin used for serial data bus SDA
    .scl_io_num = Config->I2C_ESP32_SCL_IO, //GPIO pin used for serial clock bus SCL
    .clk_source = I2C_CLK_SRC_DEFAULT, //Clock source
    .glitch_ignore_cnt = 7, //Glitch period of master bus, if the glitch period on the line is less than this value,
                            //it can be filtered out, typically value is 7
    .flags.enable_internal_pullup = true //enable/disable internal pullup resistors
    };

    esp_err_t err = i2c_new_master_bus(&BusConfig, &Config->I2C_ESP32_BusHandle); //Creating new bus, writing bus handle to &Config->BusHandle
    if (err != ESP_OK)
    {
        return ESP_FAIL;
    }

    i2c_device_config_t DeviceConfig = // structure describing device to add to a bus
    { 
    .dev_addr_length = I2C_ADDR_BIT_LEN_7, //Device's address lenght (for bme280 its 7 - 7 adress bits + 1 read/write bit)
    .device_address = Config->I2C_ESP32_Address, //For bme280 0x76 or 0x77
    .scl_speed_hz = Config->I2C_ESP32_SpeedHz //BME280 supports 3.4MHz I2C communication, ESP-IDF says it shoult NOT be larger than 400KHz
    };

    //Creating new device on bus, writing device handle to &Config->DeviceHandle
    err = i2c_master_bus_add_device(Config->I2C_ESP32_BusHandle, &DeviceConfig, &Config->I2C_ESP32_DeviceHandle);
    if (err != ESP_OK)
    {
        return ESP_FAIL;
    }

    Device->InterfacePtr = Config;                  //Connecting interface to driver
    Device->driver.ReadReg = ESP32_I2C_ReadReg;
    Device->driver.WriteReg = ESP32_I2C_WriteReg;
    Device->driver.DelayMs = ESP32_DelayMs;
}

