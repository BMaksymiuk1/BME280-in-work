//
// Created by barte on 5.09.2026.
//a

#include <stdint.h>

#include "bme280.h"
#include "bme280_driver.h"

#define BIT0 (1<<0)
#define BIT1 (1<<1)
#define BIT2 (1<<2)
#define BIT3 (1<<3)
#define BIT4 (1<<4)
#define BIT5 (1<<5)
#define BIT6 (1<<6)
#define BIT7 (1<<7)

#define CTRL_MEAS_REG 0xF4
#define CTRL_HUM_REG 0xF2
#define CONFIG_REG 0xF5
#define RESET_REG 0xE0
#define ID_REG 0xD0
#define CALIBRATION_DATA_REG1 0x88
#define CALIBRATION_DATA_REG2 0xE1
#define RAW_DATA_BUFFER_SIZE 8
#define RAW_DATA_REGISTER_ADDRESS 0xF7
#define STATUS_REG_ADDRESS 0xF3

/*Before using any of the functions, initialize logic of each sensor with BME280_Init(BME280_Device_t *Device), before this 
interface must be initialized */



// Temperature oversampling set
BME280_Status_t TemperatureOversamplingSet(BME280_Device_t *Device, BME280_Oversampling_t osrs_t)
{
    if(Device != NULL)
    {
        uint8_t Ctrl_Meas;
        int8_t status;
        if (osrs_t != OSRS_SKIP && osrs_t != OSRS_1X && osrs_t != OSRS_2X && osrs_t != OSRS_4X && osrs_t != OSRS_8X && osrs_t != OSRS_16X)
        {
            return BME280_INVALID_PARAM; // Return invalid parameter error if osrs_t is not valid
            
        }
        status = Device->driver.ReadReg(Device->InterfacePtr, CTRL_MEAS_REG, &Ctrl_Meas, sizeof(uint8_t)); // Reading from ctrl_meas register
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if reading fails
        }
        Ctrl_Meas &= ~(BIT7 | BIT6 | BIT5); // Clearing
        Ctrl_Meas |= (osrs_t<<5);    // Setting bits in ctrl_meas register
        status = Device->driver.WriteReg(Device->InterfacePtr, CTRL_MEAS_REG, Ctrl_Meas); // Writing to ctrl_meas register
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if writing fails
        }
        return BME280_OK; // Return success
    }
    return BME280_NULL_PTR; // Return NULL pointer error if Device is NULL
}

// Pressure oversampling set
BME280_Status_t PressureOversamplingSet(BME280_Device_t *Device, BME280_Oversampling_t osrs_p)
{
    if(Device != NULL)
    {
        uint8_t Ctrl_Meas, status;
        if (osrs_p != OSRS_SKIP && osrs_p != OSRS_1X && osrs_p != OSRS_2X && osrs_p != OSRS_4X && osrs_p != OSRS_8X && osrs_p != OSRS_16X)
        {
            return BME280_INVALID_PARAM; // Return invalid parameter error if osrs_t is not valid
            
        }
        status = Device->driver.ReadReg(Device->InterfacePtr, CTRL_MEAS_REG, &Ctrl_Meas, sizeof(uint8_t)); // Reading from ctrl_meas register
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if reading fails
        }
        Ctrl_Meas &= ~(BIT4 | BIT3 | BIT2); // Clearing
        Ctrl_Meas |= (osrs_p<<2);  // Setting bits in ctrl_meas register
        status =Device->driver.WriteReg(Device->InterfacePtr, CTRL_MEAS_REG, Ctrl_Meas); // Writing to ctrl_meas register
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if writing fails
        }
        return BME280_OK; // Return success
    }
    return BME280_NULL_PTR; // Return NULL pointer error if Device is NULL
}

// Mode set
BME280_Status_t ModeSet(BME280_Device_t *Device, BME280_Mode_t Mode)
{
    if(Device != NULL)
    {
        uint8_t Ctrl_Meas;
        int8_t status;
        status = Device->driver.ReadReg(Device->InterfacePtr, CTRL_MEAS_REG, &Ctrl_Meas, sizeof(uint8_t)); // Reading from ctrl_meas register
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if reading fails
        }
        if (Mode != BME280_SLEEP_MODE && Mode != BME280_FORCED_MODE && Mode != BME280_NORMAL_MODE)
            {
                return BME280_INVALID_PARAM;
            }
        Ctrl_Meas &= ~(BIT1 | BIT0); // Clearing
        Ctrl_Meas |= Mode;  // Setting bits in ctrl_meas register
        status = Device->driver.WriteReg(Device->InterfacePtr, CTRL_MEAS_REG, Ctrl_Meas); // Writing to ctrl_meas register
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if writing fails
        }
        return BME280_OK; // Return success
    }
    return BME280_NULL_PTR; // Return NULL pointer error if Device is NULL
}

// Humidity oversampling set

BME280_Status_t HumidityOversamplingSet(BME280_Device_t *Device, BME280_Oversampling_t osrs_h)
{
    if(Device !=NULL)
    {
        uint8_t Ctrl_Hum;
        int8_t status;
        if (osrs_h != OSRS_SKIP && osrs_h != OSRS_1X && osrs_h != OSRS_2X && osrs_h != OSRS_4X && osrs_h != OSRS_8X && osrs_h != OSRS_16X)
        {
            return BME280_INVALID_PARAM; // Return invalid parameter error if osrs_t is not valid
            
        }
        status = Device->driver.ReadReg(Device->InterfacePtr, CTRL_HUM_REG, &Ctrl_Hum, sizeof(uint8_t)); // Reading from ctrl_hum register
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if reading fails
        }
        Ctrl_Hum &= ~(BIT2 | BIT1 | BIT0); // Clearing
        Ctrl_Hum |= osrs_h;  // Setting bits in ctrl_hum register
        status = Device->driver.WriteReg(Device->InterfacePtr, CTRL_HUM_REG, Ctrl_Hum); // Writing to ctrl_hum register
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if writing fails
        }
        uint8_t Ctrl_Meas; // Without writing to ctrl_meas register ctrl_hum register will not update
        status = Device->driver.ReadReg(Device->InterfacePtr, CTRL_MEAS_REG, &Ctrl_Meas, sizeof(uint8_t));
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if reading fails
        }
        status = Device->driver.WriteReg(Device->InterfacePtr, CTRL_MEAS_REG, Ctrl_Meas);
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if writing fails
        }
        return BME280_OK; // Return success
    }
    return BME280_NULL_PTR; // Return NULL pointer error if Device is NULL
}

//Standby time set
BME280_Status_t StandbyTimeSet(BME280_Device_t *Device, uint8_t t_sb)
{
    if (Device != NULL)
    {
        int status;
        if (t_sb > 7)
        {
            t_sb = 7;
        }
        uint8_t Config;
        status = Device->driver.ReadReg(Device->InterfacePtr, CONFIG_REG, &Config, sizeof(uint8_t)); // Reading from config register
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if reading fails
        }
        Config &= ~(BIT7 | BIT6 | BIT5); // clearing
        Config |= (t_sb<<5); //setting
        status = Device->driver.WriteReg(Device->InterfacePtr, CONFIG_REG, Config); // Writing to config register
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if writing fails
        }
        // register does NOT update in normal mode!!
        return BME280_OK; // Return success
    }
    return BME280_NULL_PTR; // Return NULL pointer error if Device is NULL
}

//Filter time constant set
BME280_Status_t FilterSet(BME280_Device_t *Device, uint8_t filter)
{
    if (Device != NULL)
    {
        if (filter > 4)
        {
            filter = 4;
        }
        uint8_t Config;
        int8_t status;
        status = Device->driver.ReadReg(Device->InterfacePtr, CONFIG_REG, &Config, sizeof(uint8_t)); // Reading from config register
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if reading fails
        }
        Config &= ~(BIT4 | BIT3 | BIT2); // clearing
        Config |= (filter<<2); //setting
        status = Device->driver.WriteReg(Device->InterfacePtr, CONFIG_REG, Config); // Writing to config register
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if writing fails
        }
        // register does NOT update in normal mode!!
        return BME280_OK; // Return success
    }
    return BME280_NULL_PTR; // Return NULL pointer error if Device is NULL
}

//SPI 3-wire interface enable
BME280_Status_t SPI3WireEnable(BME280_Device_t *Device, uint8_t spi3w_en)
{
    if (Device != NULL)
    {
        if (spi3w_en > 1)
        {
            spi3w_en = 1;
        }
        uint8_t Config;
        int8_t status;
        status = Device->driver.ReadReg(Device->InterfacePtr, CONFIG_REG, &Config, sizeof(uint8_t)); // Reading from config register
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if reading fails
        }
        Config &= ~(BIT0); // clearing
        Config |= spi3w_en; //setting
        status = Device->driver.WriteReg(Device->InterfacePtr, CONFIG_REG, Config); // Writing to config register
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if writing fails
        }
        // register does NOT update in normal mode!!
        return BME280_OK; // Return success
    }
    return BME280_NULL_PTR; // Return NULL pointer error if Device is NULL
}

BME280_Status_t SoftReset(BME280_Device_t *Device)
{
    if (Device != NULL)
    {
        int8_t status;
        status = Device->driver.WriteReg(Device->InterfacePtr, RESET_REG, 0xB6);
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if writing fails
        }
        Device->driver.DelayMs(5); // Delay needed before further operations after soft reset
        return BME280_OK; // Return success
    }
    return BME280_NULL_PTR; // Return NULL pointer error if Device is NULL
}

BME280_Status_t IdRead(BME280_Device_t *Device)
{
    if (Device == NULL)
    {
        return BME280_NULL_PTR;
    }

    int8_t status;

    status = Device->driver.ReadReg(
        Device->InterfacePtr,
        ID_REG,
        &Device->DeviceId,
        sizeof(uint8_t)
    );

    if (status != 0)
    {
        return BME280_COMM_FAIL;
    }

    return BME280_OK;
}

BME280_Status_t ReadCalibrationData(BME280_Device_t *Device)
{
    if (Device != NULL)
    {
        uint8_t Buffer1[26];
        uint8_t Buffer2[7];
        int8_t status;
        status = Device->driver.ReadReg(Device->InterfacePtr, CALIBRATION_DATA_REG1, Buffer1, sizeof(Buffer1)); //read calibration data
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if reading fails
        }
        status = Device->driver.ReadReg(Device->InterfacePtr, CALIBRATION_DATA_REG2, Buffer2, sizeof(Buffer2));
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if reading fails
        }
        //Set calib data to structure
        //Temperature
        Device->CalibrationData.dig_T1 = (uint16_t)((Buffer1[1]<<8) | Buffer1[0]);
        Device->CalibrationData.dig_T2 = (int16_t)((Buffer1[3]<<8) | Buffer1[2]);
        Device->CalibrationData.dig_T3 = (int16_t)((Buffer1[5]<<8) | Buffer1[4]);
        //Pressure
        Device->CalibrationData.dig_P1 = (uint16_t)((Buffer1[7]<<8) | Buffer1[6]);
        Device->CalibrationData.dig_P2 = (int16_t)((Buffer1[9]<<8) | Buffer1[8]);
        Device->CalibrationData.dig_P3 = (int16_t)((Buffer1[11]<<8) | Buffer1[10]);
        Device->CalibrationData.dig_P4 = (int16_t)((Buffer1[13]<<8) | Buffer1[12]);
        Device->CalibrationData.dig_P5 = (int16_t)((Buffer1[15]<<8) | Buffer1[14]);
        Device->CalibrationData.dig_P6 = (int16_t)((Buffer1[17]<<8) | Buffer1[16]);
        Device->CalibrationData.dig_P7 = (int16_t)((Buffer1[19]<<8) | Buffer1[18]);
        Device->CalibrationData.dig_P8 = (int16_t)((Buffer1[21]<<8) | Buffer1[20]);
        Device->CalibrationData.dig_P9 = (int16_t)((Buffer1[23]<<8) | Buffer1[22]);
        //Humidity
        Device->CalibrationData.dig_H1 = (uint8_t)Buffer1[25]; //Ignore empty byte Buffer[24]
        Device->CalibrationData.dig_H2 = (int16_t)((Buffer2[1]<<8) | Buffer2[0]);
        Device->CalibrationData.dig_H3 = (uint8_t)Buffer2[2];
        // Register 0xE5 is divided between dig_H4 and dig_H5
        int16_t dig_H4 = (int16_t)((Buffer2[3]<<4) | (Buffer2[4] & (BIT0 | BIT1 | BIT2 | BIT3)));
        if(dig_H4 & 0x800) { // Check if the sign bit is set
            dig_H4 |= 0xF000; // Sign-extend to 16 bits
        }
        Device->CalibrationData.dig_H4 = dig_H4;
        int16_t dig_H5 = (int16_t)(((Buffer2[5]<<4)) | ((Buffer2[4] & (BIT7 | BIT6 | BIT5 | BIT4))>>4));
        if(dig_H5 & 0x800) { // Check if the sign bit is set
            dig_H5 |= 0xF000; // Sign-extend to 16 bits
        }
        Device->CalibrationData.dig_H5 = dig_H5;
        Device->CalibrationData.dig_H6 = (int8_t)Buffer2[6];
        return BME280_OK; // Return success
    }
    return BME280_NULL_PTR; // Return NULL pointer error if Device is NULL
}

//Reading raw data
static BME280_Status_t ReadRawData(BME280_Device_t *Device, RawData_t *RawDataPtr)
{
    if (RawDataPtr != NULL && Device != NULL) {
        int8_t status;
        uint8_t Buffer[RAW_DATA_BUFFER_SIZE];
        status = Device->driver.ReadReg(Device->InterfacePtr,RAW_DATA_REGISTER_ADDRESS, Buffer, sizeof(Buffer));
        if (status != 0)
        {
            return BME280_COMM_FAIL; // Return communication failure if reading fails
        }
        RawDataPtr->RawPressure = (int32_t)(((int32_t)Buffer[0]<<12) | (Buffer[1]<<4) | (Buffer[2]>>4));
        RawDataPtr->RawTemperature = (int32_t)(((int32_t)Buffer[3]<<12) | (Buffer[4]<<4) | (Buffer[5]>>4));
        RawDataPtr->RawHumidity = (uint16_t)(((uint16_t)Buffer[6]<<8) | Buffer[7]);
        return BME280_OK; // Return success
    }
    return BME280_NULL_PTR; // Return NULL pointer error if Device is NULL
}

uint8_t AreDataRegistersUpdated(BME280_Device_t *Device)
{
    if (Device != NULL)
    {
        uint8_t StatusBit;
        
        int8_t status;
        status = Device->driver.ReadReg(Device->InterfacePtr, STATUS_REG_ADDRESS, &StatusBit, sizeof(StatusBit));
        if (status != 0)
        {
            return 1; // Return default value if reading fails
        }
        StatusBit &= BIT3;
        if (StatusBit){return 1;} //1 when conversion is running
        else { return 0; } // 0 when results transferred to data registers
    }
    return 1; // Return 1 if Device is NULL
}

uint8_t IsInUpdate(BME280_Device_t *Device)
{
    if (Device != NULL)
    {
        uint8_t StatusBit;
        Device->driver.ReadReg(Device->InterfacePtr, STATUS_REG_ADDRESS, &StatusBit, sizeof(StatusBit));
        StatusBit &= BIT0;
        if (StatusBit){return 1;} //1 when NVM being copied to image registers
        else { return 0; } // 0 when copying is done
    }
    return 2; // Return 2 if Device is NULL
}

//Returns temperature in DegC, resolution is 0.01 DegC. Output value of "5123" equals 51.23 DegC.
static BME280_Status_t TemperatureCompensation(BME280_Device_t *Device, int32_t RawTemperature, int32_t *TemperaturePtr)
{
    if (Device != NULL && TemperaturePtr != NULL) 
    {
        int32_t Var1, Var2, T;
        Var1 = ((((RawTemperature>>3) - ((int32_t)Device->CalibrationData.dig_T1 << 1))) * ((int32_t)Device->CalibrationData.dig_T2)) >> 11;
        Var2 = (((((RawTemperature>>4) - ((int32_t)Device->CalibrationData.dig_T1)) *
            ((RawTemperature>>4) - ((int32_t)Device->CalibrationData.dig_T1))) >>12) * ((int32_t)Device->CalibrationData.dig_T3)) >> 14;
        Device->t_fine = Var1 + Var2; //High resolution temperature for further calculations
        T = (Device->t_fine * 5 + 128) >> 8; //Output temperature
        *TemperaturePtr = T;
        return BME280_OK;
    }
    return BME280_NULL_PTR; // Return NULL pointer error 
}

//Compensation algorithm comes from bme280 datasheet

//Returns pressure in Pa as uint32_t in Q24.8 format (24 int bits and 8 fractional bits)
//Output value of "24674867" represents 24674867/256 = 96386.2Pa = 963.862hPa
static BME280_Status_t PressureCompensation(BME280_Device_t *Device, int32_t RawPressure, uint32_t *PressurePtr)
{
    if (Device != NULL && PressurePtr != NULL)
    {
        int64_t var1, var2, p;
        var1 = ((int64_t)Device->t_fine) - 128000;
        var2 = var1 * var1 * (int64_t)Device->CalibrationData.dig_P6;
        var2 = var2 + ((var1 * (int64_t)Device->CalibrationData.dig_P5)<<17);
        var2 = var2 + (((int64_t)Device->CalibrationData.dig_P4)<<35);
        var1 = ((var1 * var1 *(int64_t)Device->CalibrationData.dig_P3)>>8) + ((var1 * (int64_t)Device->CalibrationData.dig_P2)<<12);
        var1 = (((((int64_t)1)<<47) + var1)) * ((int64_t)Device->CalibrationData.dig_P1)>>33;
        if (var1 == 0)
        {
            return 0; //avoid div by 0
        }
        p = 1048576 - RawPressure;
        p = (((p<<31) - var2) * 3125) / var1;
        var1 = (((int64_t)Device->CalibrationData.dig_P9) * (p>>13) * (p>>13)) >>25;
        var2 = (((int64_t)Device->CalibrationData.dig_P8) * p) >> 19;
        p = ((p + var1 + var2)>>8) + (((int64_t)Device->CalibrationData.dig_P7)<<4);
        *PressurePtr = (uint32_t)p;
        return BME280_OK; // Return success
    }
    return BME280_NULL_PTR; // Return NULL pointer error
}

//Returns humidity in %RH as uint32_t in Q22.10 format (22 integer and 10 fractional bits)
//Output value of "47445" represents 47445/1024 = 46.333%RH
static BME280_Status_t HumidityCompensation(BME280_Device_t *Device, int32_t RawHumidity, uint32_t *HumidityPtr)
{
    if (Device != NULL)
    {
        int32_t v_x1_u32r;
        v_x1_u32r = (Device->t_fine - ((int32_t)76800));
        v_x1_u32r = (((((RawHumidity<<14) - (((int32_t)Device->CalibrationData.dig_H4)<<20) - (((int32_t)Device->CalibrationData.dig_H5) *
            v_x1_u32r)) + ((int32_t)16384))>>15) * (((((((v_x1_u32r * ((int32_t)Device->CalibrationData.dig_H6))>>10) * (((v_x1_u32r
            * ((int32_t)Device->CalibrationData.dig_H3))>>11) + ((int32_t)32768)))>>10) + ((int32_t)2097152)) *
            ((int32_t)Device->CalibrationData.dig_H2) + 8192)>>14));
        v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r>>15) * (v_x1_u32r>>15))>>7) * ((int32_t)Device->CalibrationData.dig_H1))>>4));
        v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
        v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
        *HumidityPtr = (uint32_t)(v_x1_u32r>>12);
        return BME280_OK; // Return success
    }
    return BME280_NULL_PTR; // Return NULL pointer error if Device is NULL
}

BME280_Status_t GetMeasurements(BME280_Device_t *Device, OutputData_t *OutputDataPtr)
{
    if (OutputDataPtr != NULL && Device != NULL) {
        RawData_t RawData;
        BME280_Status_t RawDataReadStatus;
        RawDataReadStatus = ReadRawData(Device, &RawData);
        if (RawDataReadStatus != BME280_OK) {
            return RawDataReadStatus;
        }
        int32_t CompensatedTemperature;
        BME280_Status_t TemperatureStatus = TemperatureCompensation(Device, RawData.RawTemperature, &CompensatedTemperature);
        if (TemperatureStatus != BME280_OK) {
            return TemperatureStatus;
        }
        OutputDataPtr->Temperature = (float)CompensatedTemperature/100; // in .C
        uint32_t CompensatedPressure;
        BME280_Status_t PressureStatus = PressureCompensation(Device, RawData.RawPressure, &CompensatedPressure);
        if (PressureStatus != BME280_OK) {
            return PressureStatus;
        }
        OutputDataPtr->Pressure = (float)CompensatedPressure/25600; // in hPa
        uint32_t CompensatedHumidity;
        BME280_Status_t HumidityStatus = HumidityCompensation(Device, RawData.RawHumidity, &CompensatedHumidity);
        if (HumidityStatus != BME280_OK) {
            return HumidityStatus;
        }
        OutputDataPtr->Humidity = (float)CompensatedHumidity/1024; // in %RH
        return BME280_OK; // Return success
    }
    return BME280_NULL_PTR; // Return NULL pointer error if Device or OutputDataPtr is NULL
}
//Logic library initialization
//interface must be initialized before calling this function, and pointer to it must be set in Device->InterfacePtr
BME280_Status_t BME280_Init(BME280_Device_t *Device) 
{
    if (Device == NULL) 
    {
        return BME280_NULL_PTR;
    }
    BME280_Status_t status;
    status = SoftReset(Device);
    if (status != BME280_OK)
    {
        return status;
    }
    status = IdRead(Device);
    if (status != BME280_OK)
    {
        return status;
    }
    if (Device->DeviceId != 0x60) {
        return BME280_INVALID_ID; // Is not a BME280 sensor
    }
    status = ReadCalibrationData(Device);
    if (status != BME280_OK)
    {
        return status;
    }
    return BME280_OK; // Success
}















