//
// Created by barte on 5.09.2026.
//

#ifndef BME280LIB_BME280_H
#define BME280LIB_BME280_H

#include "bme280_driver.h"

//structure to hold the raw sensor data
typedef struct {
    uint32_t RawPressure;
    uint32_t RawTemperature;
   uint16_t RawHumidity;
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

//structure to hold the compensated output data
typedef struct {
    float Temperature;
    float Pressure;
    float Humidity;
}OutputData_t;

typedef enum {
    BME280_OK = 0, 
    BME280_NULL_PTR = -1, // Null pointer error
    BME280_COMM_FAIL = -2, // Communication Failure (I2C/SPI read/write error)
    BME280_INVALID_ID = -3, // Invalid sensor ID
    BME280_INVALID_PARAM = -4, // Invalid parameter passed to function
    BME280_CALC_FAIL = -5, // Calculation failure (e.g. division by zero)
    BME280_INTERFACE_NOT_INITIALIZED = -6 // Interface not initialized (e.g. I2C/SPI not set up)
} BME280_Status_t;

typedef enum {
    BME280_SLEEP_MODE = 0,
    BME280_FORCED_MODE = 1,
    BME280_NORMAL_MODE = 3
} BME280_Mode_t;

typedef enum {
    BME280_OSRS_SKIP = 0,
    BME280_OSRS_1X   = 1,
    BME280_OSRS_2X   = 2,
    BME280_OSRS_4X   = 3,
    BME280_OSRS_8X   = 4,
    BME280_OSRS_16X  = 5
} BME280_Oversampling_t;

typedef enum {
    BME280_STANDBY_TIME_0_5_MS = 0,
    BME280_STANDBY_TIME_62_5_MS = 1,
    BME280_STANDBY_TIME_125_MS = 2,
    BME280_STANDBY_TIME_250_MS = 3,
    BME280_STANDBY_TIME_500_MS = 4,
    BME280_STANDBY_TIME_1000_MS = 5,
    BME280_STANDBY_TIME_10_MS = 6,
    BME280_STANDBY_TIME_20_MS = 7
} BME280_StandbyTime_t;


//structure to hold the BME280 specific device information and calibration data
typedef struct {
    BME280_Driver_t driver;         // Interface functions for reading/writing registers and delays
    CalibrationData_t CalibrationData; // Unique calibration data for the sensor
    int32_t t_fine;       
    // Pointer to a user-defined structure containing for I2C port number and address, for SPI port handle and chip select pin         // Computed temperature value
    void *InterfacePtr;   
    uint8_t DeviceId;  //Sensor chip ID, expected 0x60 for BME280    
    BME280_Oversampling_t osrs_t; // Temperature oversampling setting
    BME280_Oversampling_t osrs_p; // Pressure oversampling setting
    BME280_Oversampling_t osrs_h; // Humidity oversampling setting
} BME280_Device_t;



BME280_Status_t TemperatureOversamplingSet(BME280_Device_t *Device, BME280_Oversampling_t osrs_t);
BME280_Status_t PressureOversamplingSet(BME280_Device_t *Device, BME280_Oversampling_t osrs_p);
BME280_Status_t ModeSet(BME280_Device_t *Device, BME280_Mode_t Mode);
BME280_Status_t HumidityOversamplingSet(BME280_Device_t *Device, BME280_Oversampling_t osrs_h);
BME280_Status_t StandbyTimeSet(BME280_Device_t *Device, BME280_StandbyTime_t t_sb);
BME280_Status_t FilterSet(BME280_Device_t *Device, uint8_t filter);
BME280_Status_t SPI3WireEnable(BME280_Device_t *Device, uint8_t spi3w_en);
BME280_Status_t SoftReset(BME280_Device_t *Device);
BME280_Status_t IdRead(BME280_Device_t *Device);
BME280_Status_t ReadCalibrationData(BME280_Device_t *Device);
BME280_Status_t IsMeasuring(BME280_Device_t *Device, uint8_t *IsMeasuringPtr);
BME280_Status_t IsNVMCopying(BME280_Device_t *Device, uint8_t *IsCopyingPtr);
BME280_Status_t GetMeasurements(BME280_Device_t *Device, OutputData_t *OutputDataPtr);
BME280_Status_t BME280_Init(BME280_Device_t *Device);




#endif //BME280LIB_BME280_H

