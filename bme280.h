//
// Created by barte on 5.09.2026.
//

#ifndef BME280LIB_BME280_H
#define BME280LIB_BME280_H

#include "bme280_driver.h"

//structure to hold the raw sensor data
typedef struct {
    int32_t RawPressure;
    int32_t RawTemperature;
    int16_t RawHumidity;
} RawData_t;

//structure to hold the calibration data read from the sensor
typedef struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
}CalibrationData_t;

//structure to hold the BME280 specific device information and calibration data
typedef struct {
    BME280_Driver_t driver;         // Interface functions for reading/writing registers and delays
    CalibrationData_t CalibrationData; // Unique calibration data for the sensor
    int32_t t_fine;       
    // Pointer to a user-defined structure containing for I2C port number and address, for SPI port handle and chip select pin         // Computed temperature value
    void *InterfacePtr;         
} BME280_Device_t;

//structure to hold the compensated output data
typedef struct {
    float Temperature;
    float Pressure;
    float Humidity;
}OutputData_t;

typedef enum {
    BME280_OK              = 0,
    BME280_NULL_PTR      = -1,
    BME280_COMM_FAIL     = -2, // Błąd komunikacji I2C/SPI
    BME280_DEV_NOT_FOUND = -3,  // Złe ID czujnika
    BME280_INVALID_ID = -4 // Invalid sensor ID
} BME280_Status_t;


BME280_Status_t TemperatureOversamplingSet(BME280_Device_t *Device, uint8_t osrs_t);
BME280_Status_t PressureOversamplingSet(BME280_Device_t *Device, uint8_t osrs_p);
BME280_Status_t ModeSet(BME280_Device_t *Device, uint8_t Mode);
BME280_Status_t HumidityOversamplingSet(BME280_Device_t *Device, uint8_t osrs_h);
BME280_Status_t StandbyTimeSet(BME280_Device_t *Device, uint8_t t_sb);
BME280_Status_t FilterSet(BME280_Device_t *Device, uint8_t filter);
BME280_Status_t SPI3WireEnable(BME280_Device_t *Device, uint8_t spi3w_en);
BME280_Status_t SoftReset(BME280_Device_t *Device);
uint8_t IdRead(BME280_Device_t *Device);
BME280_Status_t ReadCalibrationData(BME280_Device_t *Device);
uint8_t AreDataRegistersUpdated(BME280_Device_t *Device);
uint8_t IsInUpdate(BME280_Device_t *Device);
BME280_Status_t GetMeasurements(BME280_Device_t *Device, OutputData_t *OutputDataPtr);
BME280_Status_t BME280_Init(BME280_Device_t *Device);




#endif //BME280LIB_BME280_H

