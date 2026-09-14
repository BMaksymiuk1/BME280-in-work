

/////////////////////////////////////////////////////   LIBRARY TESTER //////////////////////////////////////////////////////



#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "esp_log.h"

#include "bme280.h"
#include "bme280_esp32_I2C_interface.h"


#define I2C_SDA_IO          GPIO_NUM_21
#define I2C_SCL_IO          GPIO_NUM_22
#define I2C_PORT            I2C_NUM_0

#define I2C_SPEED_HZ        50000

#define BME280_I2C_ADDRESS  0x76


/* BME280 registers */
#define CTRL_MEAS_REG       0xF4
#define CTRL_HUM_REG        0xF2
#define CONFIG_REG          0xF5


/* Register masks used only by tests */
#define CTRL_MEAS_MODE_MASK     0x03
#define CTRL_MEAS_OSRS_P_MASK   0x1C
#define CTRL_MEAS_OSRS_T_MASK   0xE0

#define CTRL_HUM_OSRS_MASK      0x07

#define CONFIG_T_SB_MASK        0xE0
#define CONFIG_FILTER_MASK      0x1C
#define CONFIG_SPI3W_MASK       0x01


static const char *TAG = "BME280_TEST";


static uint32_t TestsPassed = 0;
static uint32_t TestsFailed = 0;
static uint32_t CommunicationFailures = 0;


/* ============================================================
 * Test helpers
 * ============================================================ */

static void TestPass(const char *TestName)
{
    TestsPassed++;

    ESP_LOGI(
        TAG,
        "[PASS] %s",
        TestName
    );
}


static void TestFail(
    const char *TestName,
    BME280_Status_t Status)
{
    TestsFailed++;

    ESP_LOGE(
        TAG,
        "[FAIL] %s - status: %d",
        TestName,
        Status
    );
}


static void TestCommunicationFail(
    const char *TestName,
    BME280_Status_t Status)
{
    TestsFailed++;
    CommunicationFailures++;

    ESP_LOGE(
        TAG,
        "[COMM FAIL] %s - status: %d",
        TestName,
        Status
    );
}


static void TestFailInterface(
    const char *TestName,
    I2C_ESP32_Error_t Status)
{
    TestsFailed++;

    ESP_LOGE(
        TAG,
        "[FAIL] %s - interface status: %d",
        TestName,
        Status
    );
}


static BME280_Status_t ReadRegister(
    BME280_Device_t *Device,
    uint8_t RegisterAddress,
    uint8_t *Value)
{
    if (Device == NULL || Value == NULL)
    {
        return BME280_NULL_PTR;
    }

    if (Device->driver.ReadReg == NULL)
    {
        return BME280_INTERFACE_NOT_INITIALIZED;
    }

    if (Device->driver.ReadReg(
            Device->InterfacePtr,
            RegisterAddress,
            Value,
            sizeof(uint8_t)) != 0)
    {
        return BME280_COMM_FAIL;
    }

    return BME280_OK;
}


/* ============================================================
 * Test: BME280_Init
 * ============================================================ */

static void Test_Init(BME280_Device_t *Device)
{
    BME280_Status_t Status = BME280_Init(Device);

    if (Status == BME280_OK)
    {
        TestPass("BME280_Init()");
    }
    else if (Status == BME280_COMM_FAIL)
    {
        TestCommunicationFail("BME280_Init()", Status);
    }
    else
    {
        TestFail("BME280_Init()", Status);
    }
}


/* ============================================================
 * Test: IdRead
 * ============================================================ */

static void Test_IdRead(BME280_Device_t *Device)
{
    BME280_Status_t Status = IdRead(Device);

    if (Status == BME280_COMM_FAIL)
    {
        TestCommunicationFail("IdRead()", Status);
        return;
    }

    if (Status != BME280_OK)
    {
        TestFail("IdRead()", Status);
        return;
    }

    if (Device->DeviceId != 0x60)
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] IdRead() - unexpected ID: 0x%02X",
            Device->DeviceId
        );

        return;
    }

    TestPass("IdRead()");
}


/* ============================================================
 * Test: ReadCalibrationData
 * ============================================================ */

static void Test_CalibrationData(BME280_Device_t *Device)
{
    BME280_Status_t Status =
        ReadCalibrationData(Device);

    if (Status == BME280_COMM_FAIL)
    {
        TestCommunicationFail(
            "ReadCalibrationData()",
            Status
        );

        return;
    }

    if (Status != BME280_OK)
    {
        TestFail(
            "ReadCalibrationData()",
            Status
        );

        return;
    }

    /*
     * Basic sanity check.
     */

    if (Device->CalibrationData.dig_T1 == 0 ||
        Device->CalibrationData.dig_P1 == 0)
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] ReadCalibrationData() - suspicious calibration data"
        );

        return;
    }

    TestPass("ReadCalibrationData()");
}


/* ============================================================
 * Test: SoftReset
 * ============================================================ */

static void Test_SoftReset(BME280_Device_t *Device)
{
    BME280_Status_t Status =
        SoftReset(Device);

    if (Status == BME280_COMM_FAIL)
    {
        TestCommunicationFail(
            "SoftReset()",
            Status
        );

        return;
    }

    if (Status != BME280_OK)
    {
        TestFail(
            "SoftReset()",
            Status
        );

        return;
    }

    if (Device->osrs_t != BME280_OSRS_SKIP ||
        Device->osrs_p != BME280_OSRS_SKIP ||
        Device->osrs_h != BME280_OSRS_SKIP)
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] SoftReset() - local oversampling state incorrect"
        );

        return;
    }

    TestPass("SoftReset()");
}


/* ============================================================
 * Test: DelayMs callback
 * ============================================================ */

static void Test_DelayMs(BME280_Device_t *Device)
{
    if (Device == NULL)
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] DelayMs() - Device is NULL"
        );

        return;
    }

    if (Device->driver.DelayMs == NULL)
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] DelayMs() - callback is NULL"
        );

        return;
    }

    ESP_LOGI(
        TAG,
        "Testing DelayMs() - waiting 500 ms..."
    );

    Device->driver.DelayMs(500);

    ESP_LOGI(
        TAG,
        "DelayMs() finished"
    );

    TestPass("DelayMs()");
}


/* ============================================================
 * Test: TemperatureOversamplingSet
 * ============================================================ */

static void Test_TemperatureOversampling(
    BME280_Device_t *Device)
{
    const BME280_Oversampling_t Values[] =
    {
        BME280_OSRS_SKIP,
        BME280_OSRS_1X,
        BME280_OSRS_2X,
        BME280_OSRS_4X,
        BME280_OSRS_8X,
        BME280_OSRS_16X
    };

    for (uint32_t i = 0;
         i < sizeof(Values) / sizeof(Values[0]);
         i++)
    {
        BME280_Status_t Status =
            TemperatureOversamplingSet(
                Device,
                Values[i]
            );

        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "TemperatureOversamplingSet()",
                Status
            );

            continue;
        }

        if (Status != BME280_OK)
        {
            TestFail(
                "TemperatureOversamplingSet()",
                Status
            );

            continue;
        }

        if (Device->osrs_t != Values[i])
        {
            TestsFailed++;

            ESP_LOGE(
                TAG,
                "[FAIL] TemperatureOversamplingSet() - local state mismatch"
            );

            continue;
        }

        uint8_t CtrlMeas = 0;

        Status =
            ReadRegister(
                Device,
                CTRL_MEAS_REG,
                &CtrlMeas
            );

        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "TemperatureOversamplingSet() - register verification",
                Status
            );

            continue;
        }

        if (Status != BME280_OK ||
            ((CtrlMeas & CTRL_MEAS_OSRS_T_MASK) >> 5) != Values[i])
        {
            TestsFailed++;

            ESP_LOGE(
                TAG,
                "[FAIL] TemperatureOversamplingSet() - register mismatch"
            );

            continue;
        }

        TestPass("TemperatureOversamplingSet()");
    }

    /*
     * Invalid parameter
     */

    BME280_Status_t Status =
        TemperatureOversamplingSet(
            Device,
            (BME280_Oversampling_t)6
        );

    if (Status == BME280_INVALID_PARAM)
    {
        TestPass(
            "TemperatureOversamplingSet() invalid parameter"
        );
    }
    else
    {
        TestFail(
            "TemperatureOversamplingSet() invalid parameter",
            Status
        );
    }
}


/* ============================================================
 * Test: PressureOversamplingSet
 * ============================================================ */

static void Test_PressureOversampling(
    BME280_Device_t *Device)
{
    const BME280_Oversampling_t Values[] =
    {
        BME280_OSRS_SKIP,
        BME280_OSRS_1X,
        BME280_OSRS_2X,
        BME280_OSRS_4X,
        BME280_OSRS_8X,
        BME280_OSRS_16X
    };

    for (uint32_t i = 0;
         i < sizeof(Values) / sizeof(Values[0]);
         i++)
    {
        BME280_Status_t Status =
            PressureOversamplingSet(
                Device,
                Values[i]
            );

        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "PressureOversamplingSet()",
                Status
            );

            continue;
        }

        if (Status != BME280_OK)
        {
            TestFail(
                "PressureOversamplingSet()",
                Status
            );

            continue;
        }

        if (Device->osrs_p != Values[i])
        {
            TestsFailed++;

            ESP_LOGE(
                TAG,
                "[FAIL] PressureOversamplingSet() - local state mismatch"
            );

            continue;
        }

        uint8_t CtrlMeas = 0;

        Status =
            ReadRegister(
                Device,
                CTRL_MEAS_REG,
                &CtrlMeas
            );

        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "PressureOversamplingSet() - register verification",
                Status
            );

            continue;
        }

        if (Status != BME280_OK ||
            ((CtrlMeas & CTRL_MEAS_OSRS_P_MASK) >> 2) != Values[i])
        {
            TestsFailed++;

            ESP_LOGE(
                TAG,
                "[FAIL] PressureOversamplingSet() - register mismatch"
            );

            continue;
        }

        TestPass("PressureOversamplingSet()");
    }

    /*
     * Invalid parameter
     */

    BME280_Status_t Status =
        PressureOversamplingSet(
            Device,
            (BME280_Oversampling_t)6
        );

    if (Status == BME280_INVALID_PARAM)
    {
        TestPass(
            "PressureOversamplingSet() invalid parameter"
        );
    }
    else
    {
        TestFail(
            "PressureOversamplingSet() invalid parameter",
            Status
        );
    }
}


/* ============================================================
 * Test: HumidityOversamplingSet
 * ============================================================ */

static void Test_HumidityOversampling(
    BME280_Device_t *Device)
{
    const BME280_Oversampling_t Values[] =
    {
        BME280_OSRS_SKIP,
        BME280_OSRS_1X,
        BME280_OSRS_2X,
        BME280_OSRS_4X,
        BME280_OSRS_8X,
        BME280_OSRS_16X
    };

    for (uint32_t i = 0;
         i < sizeof(Values) / sizeof(Values[0]);
         i++)
    {
        BME280_Status_t Status =
            HumidityOversamplingSet(
                Device,
                Values[i]
            );

        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "HumidityOversamplingSet()",
                Status
            );

            continue;
        }

        if (Status != BME280_OK)
        {
            TestFail(
                "HumidityOversamplingSet()",
                Status
            );

            continue;
        }

        if (Device->osrs_h != Values[i])
        {
            TestsFailed++;

            ESP_LOGE(
                TAG,
                "[FAIL] HumidityOversamplingSet() - local state mismatch"
            );

            continue;
        }

        uint8_t CtrlHum = 0;

        Status =
            ReadRegister(
                Device,
                CTRL_HUM_REG,
                &CtrlHum
            );

        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "HumidityOversamplingSet() - register verification",
                Status
            );

            continue;
        }

        if (Status != BME280_OK ||
            (CtrlHum & CTRL_HUM_OSRS_MASK) != Values[i])
        {
            TestsFailed++;

            ESP_LOGE(
                TAG,
                "[FAIL] HumidityOversamplingSet() - register mismatch"
            );

            continue;
        }

        TestPass("HumidityOversamplingSet()");
    }

    /*
     * Invalid parameter
     */

    BME280_Status_t Status =
        HumidityOversamplingSet(
            Device,
            (BME280_Oversampling_t)6
        );

    if (Status == BME280_INVALID_PARAM)
    {
        TestPass(
            "HumidityOversamplingSet() invalid parameter"
        );
    }
    else
    {
        TestFail(
            "HumidityOversamplingSet() invalid parameter",
            Status
        );
    }
}


/* ============================================================
 * Test: ModeSet
 * ============================================================ */

static void Test_ModeSet(BME280_Device_t *Device)
{
    const BME280_Mode_t Modes[] =
    {
        BME280_SLEEP_MODE,
        BME280_FORCED_MODE,
        BME280_NORMAL_MODE
    };

    for (uint32_t i = 0;
         i < sizeof(Modes) / sizeof(Modes[0]);
         i++)
    {
        BME280_Status_t Status =
            ModeSet(
                Device,
                Modes[i]
            );

        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "ModeSet()",
                Status
            );

            continue;
        }

        if (Status != BME280_OK)
        {
            TestFail(
                "ModeSet()",
                Status
            );

            continue;
        }

        uint8_t CtrlMeas = 0;

        Status =
            ReadRegister(
                Device,
                CTRL_MEAS_REG,
                &CtrlMeas
            );

        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "ModeSet() - register verification",
                Status
            );

            continue;
        }

        if (Status != BME280_OK ||
            (CtrlMeas & CTRL_MEAS_MODE_MASK) != Modes[i])
        {
            TestsFailed++;

            ESP_LOGE(
                TAG,
                "[FAIL] ModeSet() - register mismatch"
            );

            continue;
        }

        TestPass("ModeSet()");
    }

    /*
     * Invalid parameter
     */

    BME280_Status_t Status =
        ModeSet(
            Device,
            (BME280_Mode_t)2
        );

    if (Status == BME280_INVALID_PARAM)
    {
        TestPass("ModeSet() invalid parameter");
    }
    else
    {
        TestFail(
            "ModeSet() invalid parameter",
            Status
        );
    }
}


/* ============================================================
 * Test: StandbyTimeSet
 * ============================================================ */

static void Test_StandbyTimeSet(
    BME280_Device_t *Device)
{
    const BME280_StandbyTime_t Values[] =
    {
        BME280_STANDBY_TIME_0_5_MS,
        BME280_STANDBY_TIME_62_5_MS,
        BME280_STANDBY_TIME_125_MS,
        BME280_STANDBY_TIME_250_MS,
        BME280_STANDBY_TIME_500_MS,
        BME280_STANDBY_TIME_1000_MS,
        BME280_STANDBY_TIME_10_MS,
        BME280_STANDBY_TIME_20_MS
    };


    /*
     * NORMAL mode
     */

    BME280_Status_t Status =
        ModeSet(
            Device,
            BME280_NORMAL_MODE
        );

    if (Status != BME280_OK)
    {
        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "StandbyTimeSet() preparation",
                Status
            );
        }
        else
        {
            TestFail(
                "StandbyTimeSet() preparation",
                Status
            );
        }

        return;
    }


    for (uint32_t i = 0;
         i < sizeof(Values) / sizeof(Values[0]);
         i++)
    {
        Status =
            StandbyTimeSet(
                Device,
                Values[i]
            );

        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "StandbyTimeSet()",
                Status
            );

            continue;
        }

        if (Status != BME280_OK)
        {
            TestFail(
                "StandbyTimeSet()",
                Status
            );

            continue;
        }

        uint8_t Config = 0;

        Status =
            ReadRegister(
                Device,
                CONFIG_REG,
                &Config
            );

        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "StandbyTimeSet() - register verification",
                Status
            );

            continue;
        }

        if (Status != BME280_OK ||
            ((Config & CONFIG_T_SB_MASK) >> 5) != Values[i])
        {
            TestsFailed++;

            ESP_LOGE(
                TAG,
                "[FAIL] StandbyTimeSet() - register mismatch"
            );

            continue;
        }

        TestPass("StandbyTimeSet()");
    }


    /*
     * FORCED mode
     */

    Status =
        ModeSet(
            Device,
            BME280_FORCED_MODE
        );

    if (Status != BME280_OK)
    {
        TestFail(
            "StandbyTimeSet() - FORCED mode preparation",
            Status
        );

        return;
    }


    Status =
        StandbyTimeSet(
            Device,
            BME280_STANDBY_TIME_20_MS
        );

    if (Status == BME280_OK)
    {
        TestPass(
            "StandbyTimeSet() in FORCED mode"
        );
    }
    else if (Status == BME280_COMM_FAIL)
    {
        TestCommunicationFail(
            "StandbyTimeSet() in FORCED mode",
            Status
        );
    }
    else
    {
        TestFail(
            "StandbyTimeSet() in FORCED mode",
            Status
        );
    }


    /*
     * SLEEP mode
     */

    Status =
        ModeSet(
            Device,
            BME280_SLEEP_MODE
        );

    if (Status != BME280_OK)
    {
        TestFail(
            "StandbyTimeSet() - SLEEP mode preparation",
            Status
        );

        return;
    }


    Status =
        StandbyTimeSet(
            Device,
            BME280_STANDBY_TIME_10_MS
        );

    if (Status == BME280_OK)
    {
        TestPass(
            "StandbyTimeSet() in SLEEP mode"
        );
    }
    else if (Status == BME280_COMM_FAIL)
    {
        TestCommunicationFail(
            "StandbyTimeSet() in SLEEP mode",
            Status
        );
    }
    else
    {
        TestFail(
            "StandbyTimeSet() in SLEEP mode",
            Status
        );
    }


    /*
     * Invalid parameter
     */

    Status =
        StandbyTimeSet(
            Device,
            (BME280_StandbyTime_t)8
        );

    if (Status == BME280_INVALID_PARAM)
    {
        TestPass(
            "StandbyTimeSet() invalid parameter"
        );
    }
    else
    {
        TestFail(
            "StandbyTimeSet() invalid parameter",
            Status
        );
    }


    /*
     * Restore NORMAL mode.
     */

    ModeSet(
        Device,
        BME280_NORMAL_MODE
    );
}


/* ============================================================
 * Test: FilterSet
 * ============================================================ */

static void Test_FilterSet(
    BME280_Device_t *Device)
{
    /*
     * Test all valid filter values.
     */

    for (uint8_t Filter = 0;
         Filter <= 4;
         Filter++)
    {
        BME280_Status_t Status =
            FilterSet(
                Device,
                Filter
            );

        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "FilterSet()",
                Status
            );

            continue;
        }

        if (Status != BME280_OK)
        {
            TestFail(
                "FilterSet()",
                Status
            );

            continue;
        }

        uint8_t Config = 0;

        Status =
            ReadRegister(
                Device,
                CONFIG_REG,
                &Config
            );

        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "FilterSet() - register verification",
                Status
            );

            continue;
        }

        if (Status != BME280_OK ||
            ((Config & CONFIG_FILTER_MASK) >> 2) != Filter)
        {
            TestsFailed++;

            ESP_LOGE(
                TAG,
                "[FAIL] FilterSet() - register mismatch"
            );

            continue;
        }

        TestPass("FilterSet()");
    }


    /*
     * Invalid parameter
     */

    BME280_Status_t Status =
        FilterSet(
            Device,
            5
        );

    if (Status == BME280_INVALID_PARAM)
    {
        TestPass(
            "FilterSet() invalid parameter"
        );
    }
    else
    {
        TestFail(
            "FilterSet() invalid parameter",
            Status
        );
    }


    /*
     * Restore a known configuration.
     */

    FilterSet(Device, 0);
}


/* ============================================================
 * Test: SPI3WireEnable
 * ============================================================ */

static void Test_SPI3WireEnable(
    BME280_Device_t *Device)
{
    /*
     * Test disabling 3-wire mode.
     * This is safe while using I2C.
     */

    BME280_Status_t Status =
        SPI3WireEnable(
            Device,
            0
        );

    if (Status == BME280_COMM_FAIL)
    {
        TestCommunicationFail(
            "SPI3WireEnable(0)",
            Status
        );

        return;
    }

    if (Status != BME280_OK)
    {
        TestFail(
            "SPI3WireEnable(0)",
            Status
        );

        return;
    }

    uint8_t Config = 0;

    Status =
        ReadRegister(
            Device,
            CONFIG_REG,
            &Config
        );

    if (Status == BME280_COMM_FAIL)
    {
        TestCommunicationFail(
            "SPI3WireEnable(0) - register verification",
            Status
        );

        return;
    }

    if (Status != BME280_OK ||
        (Config & CONFIG_SPI3W_MASK) != 0)
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] SPI3WireEnable(0) - register mismatch"
        );

        return;
    }

    TestPass("SPI3WireEnable(0)");


    /*
     * Invalid parameter.
     */

    Status =
        SPI3WireEnable(
            Device,
            2
        );

    if (Status == BME280_INVALID_PARAM)
    {
        TestPass(
            "SPI3WireEnable() invalid parameter"
        );
    }
    else
    {
        TestFail(
            "SPI3WireEnable() invalid parameter",
            Status
        );
    }


    /*
     * IMPORTANT:
     *
     * We deliberately do NOT call SPI3WireEnable(Device, 1).
     * That would switch the sensor away from I2C and prevent
     * the remaining I2C-based tests from working.
     *
     * The function's actual enable operation will be tested
     * later in a dedicated final test.
     */
}


/* ============================================================
 * Test: IsMeasuring
 * ============================================================ */

static void Test_IsMeasuring(
    BME280_Device_t *Device)
{
    uint8_t Measuring = 0xFF;

    BME280_Status_t Status =
        IsMeasuring(
            Device,
            &Measuring
        );

    if (Status == BME280_COMM_FAIL)
    {
        TestCommunicationFail(
            "IsMeasuring()",
            Status
        );

        return;
    }

    if (Status != BME280_OK)
    {
        TestFail(
            "IsMeasuring()",
            Status
        );

        return;
    }

    if (Measuring > 1)
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] IsMeasuring() - invalid return value"
        );

        return;
    }

    ESP_LOGI(
        TAG,
        "IsMeasuring() returned: %u",
        Measuring
    );

    TestPass("IsMeasuring()");
}


/* ============================================================
 * Test: IsNVMCopying
 * ============================================================ */

static void Test_IsNVMCopying(
    BME280_Device_t *Device)
{
    uint8_t Copying = 0xFF;

    BME280_Status_t Status =
        IsNVMCopying(
            Device,
            &Copying
        );

    if (Status == BME280_COMM_FAIL)
    {
        TestCommunicationFail(
            "IsNVMCopying()",
            Status
        );

        return;
    }

    if (Status != BME280_OK)
    {
        TestFail(
            "IsNVMCopying()",
            Status
        );

        return;
    }

    if (Copying > 1)
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] IsNVMCopying() - invalid return value"
        );

        return;
    }

    ESP_LOGI(
        TAG,
        "IsNVMCopying() returned: %u",
        Copying
    );

    TestPass("IsNVMCopying()");
}


/* ============================================================
 * Test: GetMeasurements
 * ============================================================ */

static void Test_GetMeasurements(
    BME280_Device_t *Device)
{
    OutputData_t Output =
    {
        .Temperature = 123.0f,
        .Pressure = 456.0f,
        .Humidity = 789.0f
    };


    /* --------------------------------------------------------
     * All measurements skipped
     * -------------------------------------------------------- */

    Device->osrs_t = BME280_OSRS_SKIP;
    Device->osrs_p = BME280_OSRS_SKIP;
    Device->osrs_h = BME280_OSRS_SKIP;

    BME280_Status_t Status =
        GetMeasurements(
            Device,
            &Output
        );

    if (Status != BME280_OK)
    {
        TestFail(
            "GetMeasurements() - all SKIP",
            Status
        );
    }
    else if (Output.Temperature != 123.0f ||
             Output.Pressure != 456.0f ||
             Output.Humidity != 789.0f)
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] GetMeasurements() - all SKIP modified output"
        );
    }
    else
    {
        TestPass(
            "GetMeasurements() - all SKIP"
        );
    }


    /* --------------------------------------------------------
     * Pressure and humidity enabled without temperature
     * -------------------------------------------------------- */

    Device->osrs_t = BME280_OSRS_SKIP;
    Device->osrs_p = BME280_OSRS_16X;
    Device->osrs_h = BME280_OSRS_1X;

    Status =
        GetMeasurements(
            Device,
            &Output
        );

    if (Status ==
        BME280_CANNOT_COMPENSATE_WITHOUT_TEMPERATURE_MEASUREMENT)
    {
        TestPass(
            "GetMeasurements() - P/H without temperature"
        );
    }
    else
    {
        TestFail(
            "GetMeasurements() - P/H without temperature",
            Status
        );
    }


    /* --------------------------------------------------------
     * Enable temperature
     * -------------------------------------------------------- */

    Status =
        TemperatureOversamplingSet(
            Device,
            BME280_OSRS_2X
        );

    if (Status != BME280_OK)
    {
        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "GetMeasurements() preparation - temperature",
                Status
            );
        }
        else
        {
            TestFail(
                "GetMeasurements() preparation - temperature",
                Status
            );
        }

        return;
    }


    /* --------------------------------------------------------
     * Pressure
     * -------------------------------------------------------- */

    Status =
        PressureOversamplingSet(
            Device,
            BME280_OSRS_16X
        );

    if (Status != BME280_OK)
    {
        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "GetMeasurements() preparation - pressure",
                Status
            );
        }
        else
        {
            TestFail(
                "GetMeasurements() preparation - pressure",
                Status
            );
        }

        return;
    }


    /* --------------------------------------------------------
     * Humidity
     * -------------------------------------------------------- */

    Status =
        HumidityOversamplingSet(
            Device,
            BME280_OSRS_1X
        );

    if (Status != BME280_OK)
    {
        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "GetMeasurements() preparation - humidity",
                Status
            );
        }
        else
        {
            TestFail(
                "GetMeasurements() preparation - humidity",
                Status
            );
        }

        return;
    }


    /* --------------------------------------------------------
     * Normal mode
     * -------------------------------------------------------- */

    Status =
        ModeSet(
            Device,
            BME280_NORMAL_MODE
        );

    if (Status != BME280_OK)
    {
        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "GetMeasurements() preparation - normal mode",
                Status
            );
        }
        else
        {
            TestFail(
                "GetMeasurements() preparation - normal mode",
                Status
            );
        }

        return;
    }


    /* --------------------------------------------------------
     * Wait until conversion is finished.
     * -------------------------------------------------------- */

    uint8_t Measuring = 1;

    for (uint32_t i = 0; i < 100; i++)
    {
        Status =
            IsMeasuring(
                Device,
                &Measuring
            );

        if (Status != BME280_OK)
        {
            if (Status == BME280_COMM_FAIL)
            {
                TestCommunicationFail(
                    "GetMeasurements() - IsMeasuring",
                    Status
                );
            }
            else
            {
                TestFail(
                    "GetMeasurements() - IsMeasuring",
                    Status
                );
            }

            return;
        }

        if (Measuring == 0)
        {
            break;
        }

        Device->driver.DelayMs(20);
    }


    /* --------------------------------------------------------
     * Read actual measurements.
     * -------------------------------------------------------- */

    Output.Temperature = 0.0f;
    Output.Pressure = 0.0f;
    Output.Humidity = 0.0f;


    Status =
        GetMeasurements(
            Device,
            &Output
        );

    if (Status == BME280_COMM_FAIL)
    {
        TestCommunicationFail(
            "GetMeasurements() - real measurement",
            Status
        );

        return;
    }

    if (Status != BME280_OK)
    {
        TestFail(
            "GetMeasurements() - real measurement",
            Status
        );

        return;
    }


    ESP_LOGI(
        TAG,
        "Temperature: %.2f C",
        Output.Temperature
    );

    ESP_LOGI(
        TAG,
        "Pressure: %.2f hPa",
        Output.Pressure
    );

    ESP_LOGI(
        TAG,
        "Humidity: %.2f %%RH",
        Output.Humidity
    );


    /* --------------------------------------------------------
     * Basic plausibility checks.
     * -------------------------------------------------------- */

    if (Output.Temperature < -40.0f ||
        Output.Temperature > 85.0f)
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] GetMeasurements() - temperature out of range"
        );

        return;
    }


    if (Output.Pressure < 300.0f ||
        Output.Pressure > 1200.0f)
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] GetMeasurements() - pressure out of range"
        );

        return;
    }


    if (Output.Humidity < 0.0f ||
        Output.Humidity > 100.0f)
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] GetMeasurements() - humidity out of range"
        );

        return;
    }


    TestPass(
        "GetMeasurements() - real measurement"
    );
}


/* ============================================================
 * Test: NULL pointer handling
 * ============================================================ */

static void Test_NullPointers(
    BME280_Device_t *Device)
{
    if (TemperatureOversamplingSet(
            NULL,
            BME280_OSRS_1X) == BME280_NULL_PTR)
    {
        TestPass(
            "TemperatureOversamplingSet(NULL)"
        );
    }
    else
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] TemperatureOversamplingSet(NULL)"
        );
    }


    if (PressureOversamplingSet(
            NULL,
            BME280_OSRS_1X) == BME280_NULL_PTR)
    {
        TestPass(
            "PressureOversamplingSet(NULL)"
        );
    }
    else
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] PressureOversamplingSet(NULL)"
        );
    }


    if (HumidityOversamplingSet(
            NULL,
            BME280_OSRS_1X) == BME280_NULL_PTR)
    {
        TestPass(
            "HumidityOversamplingSet(NULL)"
        );
    }
    else
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] HumidityOversamplingSet(NULL)"
        );
    }


    if (ModeSet(
            NULL,
            BME280_SLEEP_MODE) == BME280_NULL_PTR)
    {
        TestPass(
            "ModeSet(NULL)"
        );
    }
    else
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] ModeSet(NULL)"
        );
    }


    if (StandbyTimeSet(
            NULL,
            BME280_STANDBY_TIME_10_MS) == BME280_NULL_PTR)
    {
        TestPass(
            "StandbyTimeSet(NULL)"
        );
    }
    else
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] StandbyTimeSet(NULL)"
        );
    }


    if (FilterSet(
            NULL,
            0) == BME280_NULL_PTR)
    {
        TestPass(
            "FilterSet(NULL)"
        );
    }
    else
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] FilterSet(NULL)"
        );
    }


    if (SPI3WireEnable(
            NULL,
            0) == BME280_NULL_PTR)
    {
        TestPass(
            "SPI3WireEnable(NULL)"
        );
    }
    else
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] SPI3WireEnable(NULL)"
        );
    }


    if (SoftReset(NULL) == BME280_NULL_PTR)
    {
        TestPass(
            "SoftReset(NULL)"
        );
    }
    else
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] SoftReset(NULL)"
        );
    }


    if (IdRead(NULL) == BME280_NULL_PTR)
    {
        TestPass(
            "IdRead(NULL)"
        );
    }
    else
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] IdRead(NULL)"
        );
    }


    if (ReadCalibrationData(NULL) == BME280_NULL_PTR)
    {
        TestPass(
            "ReadCalibrationData(NULL)"
        );
    }
    else
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] ReadCalibrationData(NULL)"
        );
    }


    if (IsMeasuring(
            Device,
            NULL) == BME280_NULL_PTR)
    {
        TestPass(
            "IsMeasuring(NULL output)"
        );
    }
    else
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] IsMeasuring(NULL output)"
        );
    }


    if (IsNVMCopying(
            Device,
            NULL) == BME280_NULL_PTR)
    {
        TestPass(
            "IsNVMCopying(NULL output)"
        );
    }
    else
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] IsNVMCopying(NULL output)"
        );
    }


    if (GetMeasurements(
            Device,
            NULL) == BME280_NULL_PTR)
    {
        TestPass(
            "GetMeasurements(NULL output)"
        );
    }
    else
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] GetMeasurements(NULL output)"
        );
    }


    if (BME280_Init(NULL) == BME280_NULL_PTR)
    {
        TestPass(
            "BME280_Init(NULL)"
        );
    }
    else
    {
        TestsFailed++;

        ESP_LOGE(
            TAG,
            "[FAIL] BME280_Init(NULL)"
        );
    }
}


/* ============================================================
 * Final SPI3-wire test
 * ============================================================ */

static void Test_SPI3WireEnableFinal(
    BME280_Device_t *Device)
{
    /*
     * This test MUST be last because enabling SPI3W
     * disables I2C on the BME280.
     */

    BME280_Status_t Status =
        SPI3WireEnable(
            Device,
            1
        );

    if (Status == BME280_OK)
    {
        TestPass(
            "SPI3WireEnable(1) - FINAL TEST"
        );

        ESP_LOGW(
            TAG,
            "BME280 switched to SPI 3-wire mode."
        );
    }
    else if (Status == BME280_COMM_FAIL)
    {
        TestCommunicationFail(
            "SPI3WireEnable(1) - FINAL TEST",
            Status
        );
    }
    else
    {
        TestFail(
            "SPI3WireEnable(1) - FINAL TEST",
            Status
        );
    }
}


/* ============================================================
 * app_main
 * ============================================================ */

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "BME280 LIBRARY TEST START"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );


    /* ============================================================
     * Create I2C bus
     * ============================================================ */

    i2c_master_bus_config_t BusConfig =
    {
        .i2c_port = I2C_PORT,
        .sda_io_num = I2C_SDA_IO,
        .scl_io_num = I2C_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true
    };


    i2c_master_bus_handle_t BusHandle = NULL;


    esp_err_t err =
        i2c_new_master_bus(
            &BusConfig,
            &BusHandle
        );


    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to create I2C bus: %s",
            esp_err_to_name(err)
        );

        return;
    }


    ESP_LOGI(
        TAG,
        "I2C bus created"
    );


    /* ============================================================
     * Wait for hardware startup
     * ============================================================ */

    vTaskDelay(
        pdMS_TO_TICKS(1000)
    );


    /* ============================================================
     * Probe BME280
     * ============================================================ */

    err =
        i2c_master_probe(
            BusHandle,
            BME280_I2C_ADDRESS,
            1000
        );


    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "BME280 probe failed: %s",
            esp_err_to_name(err)
        );

        i2c_del_master_bus(BusHandle);

        return;
    }


    ESP_LOGI(
        TAG,
        "BME280 detected at address 0x%02X",
        BME280_I2C_ADDRESS
    );


    /* ============================================================
     * Create BME280 object
     * ============================================================ */

    BME280_Device_t Device =
    {
        0
    };


    I2C_ESP32_Config_t InterfaceConfig =
    {
        .I2C_ESP32_Address = BME280_I2C_ADDRESS,
        .I2C_ESP32_SpeedHz = I2C_SPEED_HZ,
        .I2C_ESP32_BusHandle = BusHandle,
        .I2C_ESP32_DeviceHandle = NULL
    };


    /* ============================================================
     * Initialize interface
     * ============================================================ */

    I2C_ESP32_Error_t InterfaceStatus =
        BME280_ESP32_I2C_Interface_Init(
            &Device,
            &InterfaceConfig
        );


    if (InterfaceStatus != I2C_ESP32_OK)
    {
        TestFailInterface(
            "BME280_ESP32_I2C_Interface_Init()",
            InterfaceStatus
        );

        i2c_del_master_bus(BusHandle);

        return;
    }


    TestPass(
        "BME280_ESP32_I2C_Interface_Init()"
    );


    /* ============================================================
     * Initialize BME280
     * ============================================================ */

    BME280_Status_t Status =
        BME280_Init(
            &Device
        );


    if (Status != BME280_OK)
    {
        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "BME280_Init()",
                Status
            );
        }
        else
        {
            TestFail(
                "BME280_Init()",
                Status
            );
        }

        i2c_master_bus_rm_device(
            InterfaceConfig.I2C_ESP32_DeviceHandle
        );

        i2c_del_master_bus(BusHandle);

        return;
    }


    TestPass("BME280_Init()");


    ESP_LOGI(
        TAG,
        "Device ID: 0x%02X",
        Device.DeviceId
    );


    /* ============================================================
     * Test basic functions
     * ============================================================ */

    Test_IdRead(&Device);

    Test_CalibrationData(&Device);

    Test_DelayMs(&Device);

    Test_SoftReset(&Device);


    /*
     * SoftReset resets the sensor.
     * Initialize again so all subsequent tests start
     * from a known state.
     */

    Status =
        BME280_Init(
            &Device
        );


    if (Status != BME280_OK)
    {
        if (Status == BME280_COMM_FAIL)
        {
            TestCommunicationFail(
                "BME280_Init() after SoftReset",
                Status
            );
        }
        else
        {
            TestFail(
                "BME280_Init() after SoftReset",
                Status
            );
        }

        i2c_master_bus_rm_device(
            InterfaceConfig.I2C_ESP32_DeviceHandle
        );

        i2c_del_master_bus(BusHandle);

        return;
    }


    TestPass(
        "BME280_Init() after SoftReset"
    );


    /* ============================================================
     * Configuration tests
     * ============================================================ */

    Test_TemperatureOversampling(&Device);

    Test_PressureOversampling(&Device);

    Test_HumidityOversampling(&Device);

    Test_ModeSet(&Device);

    Test_StandbyTimeSet(&Device);

    Test_FilterSet(&Device);

    Test_SPI3WireEnable(&Device);


    /* ============================================================
     * Status tests
     * ============================================================ */

    Test_IsMeasuring(&Device);

    Test_IsNVMCopying(&Device);


    /* ============================================================
     * Measurement tests
     * ============================================================ */

    Test_GetMeasurements(&Device);


    /* ============================================================
     * NULL pointer tests
     * ============================================================ */

    Test_NullPointers(&Device);


    /* ============================================================
     * Summary before final SPI test
     * ============================================================ */

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "TEST SUMMARY BEFORE FINAL SPI TEST"
    );

    ESP_LOGI(
        TAG,
        "PASSED: %lu",
        (unsigned long)TestsPassed
    );

    ESP_LOGI(
        TAG,
        "FAILED: %lu",
        (unsigned long)TestsFailed
    );

    ESP_LOGI(
        TAG,
        "COMMUNICATION FAILURES: %lu",
        (unsigned long)CommunicationFailures
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );


    /* ============================================================
     * Final SPI3-wire enable test
     * ============================================================ */

    Test_SPI3WireEnableFinal(&Device);


    /* ============================================================
     * Cleanup
     *
     * SPI3W is enabled now, but removing the ESP-IDF
     * device and bus does not require another I2C transaction.
     * ============================================================ */

    if (InterfaceConfig.I2C_ESP32_DeviceHandle != NULL)
    {
        i2c_master_bus_rm_device(
            InterfaceConfig.I2C_ESP32_DeviceHandle
        );
    }


    if (BusHandle != NULL)
    {
        i2c_del_master_bus(
            BusHandle
        );
    }


    /* ============================================================
     * Final result
     * ============================================================ */

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "FINAL TEST RESULT"
    );

    ESP_LOGI(
        TAG,
        "PASSED: %lu",
        (unsigned long)TestsPassed
    );

    ESP_LOGI(
        TAG,
        "FAILED: %lu",
        (unsigned long)TestsFailed
    );

    ESP_LOGI(
        TAG,
        "COMMUNICATION FAILURES: %lu",
        (unsigned long)CommunicationFailures
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );
}