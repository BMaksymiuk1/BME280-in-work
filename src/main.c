
#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"

#include "bme280.h"
#include "bme280_esp32_I2C_interface.h"


/* ============================================================
 * ESP32 I2C configuration
 * ============================================================ */

#define I2C_MASTER_PORT    I2C_NUM_0
#define I2C_MASTER_SDA     GPIO_NUM_21
#define I2C_MASTER_SCL     GPIO_NUM_22

/*
 * BME280 I2C address:
 *
 * SDO -> GND  : 0x76
 * SDO -> 3.3V : 0x77
 */
#define BME280_I2C_ADDRESS 0x76


/* ============================================================
 * BME280 status printing
 * ============================================================ */

static void PrintBME280Status(BME280_Status_t Status)
{
    switch (Status)
    {
        case BME280_OK:
            printf("BME280_OK\n");
            break;

        case BME280_NULL_PTR:
            printf("BME280_NULL_PTR\n");
            break;

        case BME280_COMM_FAIL:
            printf("BME280_COMM_FAIL\n");
            break;

        case BME280_INVALID_ID:
            printf("BME280_INVALID_ID\n");
            break;

        case BME280_INVALID_PARAM:
            printf("BME280_INVALID_PARAM\n");
            break;

        case BME280_CALC_FAIL:
            printf("BME280_CALC_FAIL\n");
            break;

        case BME280_INTERFACE_NOT_INITIALIZED:
            printf("BME280_INTERFACE_NOT_INITIALIZED\n");
            break;

        case BME280_CANNOT_COMPENSATE_WITHOUT_TEMPERATURE_MEASUREMENT:
            printf(
                "BME280_CANNOT_COMPENSATE_WITHOUT_TEMPERATURE_MEASUREMENT\n"
            );
            break;

        default:
            printf(
                "UNKNOWN BME280 STATUS: %d\n",
                Status
            );
            break;
    }
}


/* ============================================================
 * ESP32 I2C initialization
 * ============================================================ */

static void I2C_MasterInit(void)
{
    i2c_config_t I2C_Config =
    {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA,
        .scl_io_num = I2C_MASTER_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };

    ESP_ERROR_CHECK(
        i2c_param_config(
            I2C_MASTER_PORT,
            &I2C_Config
        )
    );

    ESP_ERROR_CHECK(
        i2c_driver_install(
            I2C_MASTER_PORT,
            I2C_MODE_MASTER,
            0,
            0,
            0
        )
    );
}


/* ============================================================
 * Main
 * ============================================================ */

void app_main(void)
{
    BME280_Status_t Status;

    printf("\n");
    printf("========================================\n");
    printf("          BME280 library test\n");
    printf("========================================\n");


    /*
     * Give the BME280 and power supply time to stabilize
     * after ESP32 power-up.
     */
    vTaskDelay(
        pdMS_TO_TICKS(200)
    );


    /* --------------------------------------------------------
     * 1. Initialize I2C
     * -------------------------------------------------------- */

    printf("\nInitializing I2C...\n");

    I2C_MasterInit();

    printf("I2C initialized.\n");


    /* --------------------------------------------------------
     * 2. Create BME280 device object
     * -------------------------------------------------------- */

    BME280_Device_t BME280_Device = {0};


    /* --------------------------------------------------------
     * 3. Create I2C configuration
     *
     * This object must remain alive while the BME280 device
     * is being used because Device->InterfacePtr points to it.
     * -------------------------------------------------------- */

    ESP32_I2C_Config_t BME280_I2C_Config =
    {
        .I2C_Port_Num = I2C_MASTER_PORT,
        .I2C_Address = BME280_I2C_ADDRESS
    };


    /* --------------------------------------------------------
     * 4. Initialize BME280 ESP32 I2C interface
     * -------------------------------------------------------- */

    printf("\nInitializing BME280 ESP32 I2C interface...\n");

    I2C_ESP32_Error_t I2C_Status =
        BME280_ESP32_I2C_Interface_Init(
            &BME280_Device,
            &BME280_I2C_Config
        );

    if (I2C_Status != I2C_ESP32_OK)
    {
        printf(
            "BME280 I2C interface initialization failed: %d\n",
            I2C_Status
        );

        return;
    }

    printf("BME280 ESP32 I2C interface initialized.\n");


    /* --------------------------------------------------------
     * 5. Soft reset
     * -------------------------------------------------------- */

    printf("\nSoftReset...\n");

    Status = SoftReset(&BME280_Device);

    if (Status != BME280_OK)
    {
        printf("SoftReset failed: ");
        PrintBME280Status(Status);

        return;
    }

    printf("SoftReset OK\n");


    /* --------------------------------------------------------
     * 6. Read device ID
     * -------------------------------------------------------- */

    printf("\nReading device ID...\n");

    Status = IdRead(&BME280_Device);

    if (Status != BME280_OK)
    {
        printf("IdRead failed: ");
        PrintBME280Status(Status);

        return;
    }

    printf(
        "Device ID: 0x%02X\n",
        BME280_Device.DeviceId
    );


    /* --------------------------------------------------------
     * 7. Verify device ID
     * -------------------------------------------------------- */

    if (BME280_Device.DeviceId != 0x60)
    {
        printf(
            "Invalid BME280 device ID: 0x%02X\n",
            BME280_Device.DeviceId
        );

        return;
    }

    printf("Device ID is valid.\n");


    /* --------------------------------------------------------
     * 8. Read calibration data
     * -------------------------------------------------------- */

    printf("\nReading calibration data...\n");

    Status = ReadCalibrationData(
        &BME280_Device
    );

    if (Status != BME280_OK)
    {
        printf("ReadCalibrationData failed: ");
        PrintBME280Status(Status);

        return;
    }

    printf("Calibration data read successfully.\n");


    printf("\n========================================\n");
    printf("       BME280 initialization OK\n");
    printf("========================================\n");


    /* --------------------------------------------------------
     * 9. Temperature oversampling
     * -------------------------------------------------------- */

    printf("\nSetting temperature oversampling...\n");

    Status = TemperatureOversamplingSet(
        &BME280_Device,
        BME280_OSRS_2X
    );

    if (Status != BME280_OK)
    {
        printf("TemperatureOversamplingSet failed: ");
        PrintBME280Status(Status);

        return;
    }

    printf("Temperature oversampling: 2X\n");


    /* --------------------------------------------------------
     * 10. Pressure oversampling
     * -------------------------------------------------------- */

    printf("\nSetting pressure oversampling...\n");

    Status = PressureOversamplingSet(
        &BME280_Device,
        BME280_OSRS_16X
    );

    if (Status != BME280_OK)
    {
        printf("PressureOversamplingSet failed: ");
        PrintBME280Status(Status);

        return;
    }

    printf("Pressure oversampling: 16X\n");


    /* --------------------------------------------------------
     * 11. Humidity oversampling
     * -------------------------------------------------------- */

    printf("\nSetting humidity oversampling...\n");

    Status = HumidityOversamplingSet(
        &BME280_Device,
        BME280_OSRS_1X
    );

    if (Status != BME280_OK)
    {
        printf("HumidityOversamplingSet failed: ");
        PrintBME280Status(Status);

        return;
    }

    printf("Humidity oversampling: 1X\n");


    /* --------------------------------------------------------
     * 12. Standby time
     * -------------------------------------------------------- */

    printf("\nSetting standby time...\n");

    Status = StandbyTimeSet(
        &BME280_Device,
        BME280_STANDBY_TIME_1000_MS
    );

    if (Status != BME280_OK)
    {
        printf("StandbyTimeSet failed: ");
        PrintBME280Status(Status);

        return;
    }

    printf("Standby time: 1000 ms\n");


    /* --------------------------------------------------------
     * 13. Filter
     * -------------------------------------------------------- */

    printf("\nSetting filter...\n");

    Status = FilterSet(
        &BME280_Device,
        2
    );

    if (Status != BME280_OK)
    {
        printf("FilterSet failed: ");
        PrintBME280Status(Status);

        return;
    }

    printf("Filter: 2\n");


    /* --------------------------------------------------------
     * 14. NORMAL mode
     * -------------------------------------------------------- */

    printf("\nSetting NORMAL mode...\n");

    Status = ModeSet(
        &BME280_Device,
        BME280_NORMAL_MODE
    );

    if (Status != BME280_OK)
    {
        printf("ModeSet failed: ");
        PrintBME280Status(Status);

        return;
    }

    printf("NORMAL mode enabled.\n");


    /* --------------------------------------------------------
     * 15. Measurement loop
     * -------------------------------------------------------- */

    printf("\nStarting measurement loop...\n");

    while (1)
    {
        uint8_t IsMeasuringValue = 0;


        /* ----------------------------------------------------
         * Check whether conversion is currently running.
         * ---------------------------------------------------- */

        Status = IsMeasuring(
            &BME280_Device,
            &IsMeasuringValue
        );

        if (Status != BME280_OK)
        {
            printf("\nIsMeasuring failed: ");
            PrintBME280Status(Status);

            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );

            continue;
        }


        /* ----------------------------------------------------
         * Read measurements only after conversion has finished.
         * ---------------------------------------------------- */

        if (IsMeasuringValue == 0)
        {
            OutputData_t OutputData;

            Status = GetMeasurements(
                &BME280_Device,
                &OutputData
            );

            if (Status != BME280_OK)
            {
                printf("\nGetMeasurements failed: ");
                PrintBME280Status(Status);
            }
            else
            {
                printf(
                    "\n----------------------------------------\n"
                );

                printf(
                    "Measuring:    %u\n",
                    IsMeasuringValue
                );

                printf(
                    "Temperature:  %.2f C\n",
                    OutputData.Temperature
                );

                printf(
                    "Pressure:     %.2f hPa\n",
                    OutputData.Pressure
                );

                printf(
                    "Humidity:     %.2f %%RH\n",
                    OutputData.Humidity
                );
            }
        }
        else
        {
            printf(
                "\nMeasurement in progress - skipping read.\n"
            );
        }


        /* ----------------------------------------------------
         * Check NVM update status.
         * ---------------------------------------------------- */

        uint8_t IsCopyingValue = 0;

        Status = IsNVMCopying(
            &BME280_Device,
            &IsCopyingValue
        );

        if (Status == BME280_OK)
        {
            printf(
                "NVM copying:  %u\n",
                IsCopyingValue
            );
        }


        /* ----------------------------------------------------
         * Wait before next iteration.
         * ---------------------------------------------------- */

        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }
}