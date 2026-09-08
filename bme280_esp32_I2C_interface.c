
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bme280_esp32_I2C_interface.h"
#include "bme280_driver.h"
#include "bme280.h"

#define I2C_TIMEOUT_MS     1000

/* Recommended to do in main.c: for each sensor create ESP32_I2C_Config_t variable, initialize it with the appropriate
 I2C port number and address, then use BME280_ESP32_I2C_Interface_Init(BME280_Device_t *Device, ESP32_I2C_Config_t *Config) */



static int8_t ESP32_I2C_ReadReg(void *InterfacePtr, uint8_t RegisterAddress, uint8_t *Buffer, uint8_t Length) 
{
    ESP32_I2C_Config_t *Config = (ESP32_I2C_Config_t *)InterfacePtr; // Cast the pointer to the configuration structure
    // i2c_master_write_read_device implements "Repeated Start"
    esp_err_t err = i2c_master_write_read_device(
        Config->I2C_Port_Num, 
        Config->I2C_Address, 
        &RegisterAddress, 1, 
        Buffer, Length, 
        I2C_TIMEOUT_MS / portTICK_PERIOD_MS
    );
    
    return (err == ESP_OK) ? 0 : -1;
}

static int8_t ESP32_I2C_WriteReg(void *InterfacePtr, uint8_t RegisterAddress, uint8_t NewByte) 
{
    ESP32_I2C_Config_t *Config = (ESP32_I2C_Config_t *)InterfacePtr; // Cast the pointer to the configuration structure
    // Packing the register address and the new byte into a single buffer for writing
    uint8_t WriteBuffer[2] = {RegisterAddress, NewByte};
    
    esp_err_t err = i2c_master_write_to_device(
        Config->I2C_Port_Num, 
        Config->I2C_Address, 
        WriteBuffer, sizeof(WriteBuffer), 
        I2C_TIMEOUT_MS / portTICK_PERIOD_MS
    );
    
    return (err == ESP_OK) ? 0 : -1;
}

static void ESP32_DelayMs(uint16_t ms) 
{
    TickType_t ticks = ms / portTICK_PERIOD_MS;
    vTaskDelay(ticks > 0 ? ticks : 1);
}



// Zmieniamy argumenty: przyjmujemy główną strukturę i adres I2C
void BME280_ESP32_I2C_Interface_Init(BME280_Device_t *Device, ESP32_I2C_Config_t *Config) 
{
    if (Device != NULL && Config != NULL) 
    {
        Device->InterfacePtr = Config; 
        Device->driver.ReadReg  = ESP32_I2C_ReadReg;
        Device->driver.WriteReg = ESP32_I2C_WriteReg;
        Device->driver.DelayMs  = ESP32_DelayMs;
    }
}