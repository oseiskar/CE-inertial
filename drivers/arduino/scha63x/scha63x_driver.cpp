#include <stdio.h>
#include "config.h"
#include "scha63x_driver.h"
#include "scha63x_spi.h"
#include "arduino_timers.h"
#include "scha63x_spi_frame.h"

/*!
    @file scha63x_driver.cpp
    @brief Driver for SCHA63X sensor

    Sensor documentation "SCHA63T-K03 Data Sheet", available on murata.com 
*/


// Macros

///@{
/*! \brief Macro for parsing values from sensor MISO words. */
#define SPI_DATA_INT8_UPPER(a) ((int8_t)(((a) >> 16) & 0xff))
#define SPI_DATA_INT8_LOWER(a) ((int8_t)(((a) >> 8) & 0xff))
#define SPI_DATA_INT16(a) ((int16_t)(((a) >> 8) & 0xffff))
#define SPI_DATA_UINT16(a) ((uint16_t)(((a) >> 8) & 0xffff))
#define SPI_DATA_CHECK_RS_ERROR(a) ((((a) >> 24) & 0x03) != 1 ? true : false) // true = RS error
#define GET_TEMPERATURE(a) (25 + ((a) / 30.0))
///@}

// Static function prototypes
static bool scha63x_check_init_due(void);
static bool scha63x_check_init_uno(void);
static bool scha63x_check_rs_error(uint32_t *data, int size);
static bool scha63x_check_rs_error_3(uint32_t miso_1, uint32_t miso_2, uint32_t miso_3);
static bool scha63x_check_rs_error_5(uint32_t miso_1, uint32_t miso_2, uint32_t miso_3,
                                     uint32_t miso_4, uint32_t miso_5);

// Internal data structures
static scha63x_cacv scha63x_cac_values; // Cross-axis compensation values


// Sensor initialization

/*!
    \brief Wrapper for scha63x_init() 
    
    Includes generate_filter_frames for filter manipulation

    \param serial_num pointer to a buffer for storing IMU's serial number
    \param config filter configuration for sensor
    \return sensor status after initialization, check header file for definitions
*/
int initialize_sensor(char *serial_num, scha63x_sensor_config *config)
{   
    generate_filter_frames(config);
    return scha63x_init(serial_num);
}


/*!
    \brief Init SCHA63X sensor

    Sensor startup sequence, documentation figure 7
    
    \param serial_num pointer to a buffer for storing IMU's serial number
*/
int scha63x_init(char *serial_num)
{
    bool status_DUE = false;
    bool status_UNO = false;
    int attempt;
    int const num_attempts = 5;

    // HW Reset (reset via SPI)
    SPI_ASIC_UNO(SPI_FRAME_WRITE_RESET);
    SPI_ASIC_DUE(SPI_FRAME_WRITE_REG_BANK_0); // Make sure we are in bank 0, otherwise SPI reset is not available.
    SPI_ASIC_DUE(SPI_FRAME_WRITE_RESET);

    // Wait 25 ms for the non-volatile memory (NVM) Read
    Wait_ms(25);

    // Read UNO asic serial number
    SPI_ASIC_UNO(SPI_FRAME_READ_TRC_2);
    uint16_t trc_2 = SPI_DATA_UINT16(SPI_ASIC_UNO(SPI_FRAME_READ_TRC_0));
    uint16_t trc_0 = SPI_DATA_UINT16(SPI_ASIC_UNO(SPI_FRAME_READ_TRC_1));
    uint16_t trc_1 = SPI_DATA_UINT16(SPI_ASIC_UNO(SPI_FRAME_READ_TRC_1));

    // Build serial number string
    uint16_t id_1 = (trc_2 >> 8) & 0x0f;
    uint16_t id_0 = trc_0 & 0xffff;
    uint16_t id_2 = trc_1 & 0xffff;
    snprintf(serial_num, 14, "%05d%01x%04X", id_2, id_1, id_0);

    // Activate DUE asic test mode to be able to read cross-axis
    // compensation values from DUE NVM.
    SPI_ASIC_DUE(SPI_FRAME_WRITE_MODE_ASM_010);
    SPI_ASIC_DUE(SPI_FRAME_READ_MODE);
    SPI_ASIC_DUE(SPI_FRAME_WRITE_MODE_ASM_001);
    SPI_ASIC_DUE(SPI_FRAME_READ_MODE);
    SPI_ASIC_DUE(SPI_FRAME_WRITE_MODE_ASM_100);
    SPI_ASIC_DUE(SPI_FRAME_READ_MODE);
    uint32_t resp = SPI_ASIC_DUE(SPI_FRAME_READ_MODE);

    // Check if device is correctly set to test mode
    if ((SPI_DATA_UINT16(resp) & 0x7) == 7)
    {

        // Read cross-axis compensation values
        SPI_ASIC_DUE(0xFC00051A);
        SPI_ASIC_DUE(0x2C0000CB);
        uint32_t cxx_cxy = SPI_ASIC_DUE(0x4C00009B);
        uint32_t cxz_cyx = SPI_ASIC_DUE(0x50000089);
        uint32_t cyy_cyz = SPI_ASIC_DUE(0x5400008F);
        uint32_t czx_czy = SPI_ASIC_DUE(0x58000085);
        uint32_t czz_bxx = SPI_ASIC_DUE(0x5C000083);
        uint32_t bxy_bxz = SPI_ASIC_DUE(0x600000A1);
        uint32_t byx_byy = SPI_ASIC_DUE(0x6C0000AB);
        uint32_t byz_bzx = SPI_ASIC_DUE(0x700000B9);
        uint32_t bzy_bzz = SPI_ASIC_DUE(0x700000B9);

        scha63x_cac_values.cxx = SPI_DATA_INT8_LOWER(cxx_cxy) / 4096.0 + 1;
        scha63x_cac_values.cxy = SPI_DATA_INT8_UPPER(cxx_cxy) / 4096.0;
        scha63x_cac_values.cxz = SPI_DATA_INT8_LOWER(cxz_cyx) / 4096.0;
        scha63x_cac_values.cyx = SPI_DATA_INT8_UPPER(cxz_cyx) / 4096.0;
        scha63x_cac_values.cyy = SPI_DATA_INT8_LOWER(cyy_cyz) / 4096.0 + 1;
        scha63x_cac_values.cyz = SPI_DATA_INT8_UPPER(cyy_cyz) / 4096.0;
        scha63x_cac_values.czx = SPI_DATA_INT8_LOWER(czx_czy) / 4096.0;
        scha63x_cac_values.czy = SPI_DATA_INT8_UPPER(czx_czy) / 4096.0;
        scha63x_cac_values.czz = SPI_DATA_INT8_LOWER(czz_bxx) / 4096.0 + 1;
        scha63x_cac_values.bxx = SPI_DATA_INT8_UPPER(czz_bxx) / 4096.0 + 1;
        scha63x_cac_values.bxy = SPI_DATA_INT8_LOWER(bxy_bxz) / 4096.0;
        scha63x_cac_values.bxz = SPI_DATA_INT8_UPPER(bxy_bxz) / 4096.0;
        scha63x_cac_values.byx = SPI_DATA_INT8_LOWER(byx_byy) / 4096.0;
        scha63x_cac_values.byy = SPI_DATA_INT8_UPPER(byx_byy) / 4096.0 + 1;
        scha63x_cac_values.byz = SPI_DATA_INT8_LOWER(byz_bzx) / 4096.0;
        scha63x_cac_values.bzx = SPI_DATA_INT8_UPPER(byz_bzx) / 4096.0;
        scha63x_cac_values.bzy = SPI_DATA_INT8_LOWER(bzy_bzz) / 4096.0;
        scha63x_cac_values.bzz = SPI_DATA_INT8_UPPER(bzy_bzz) / 4096.0 + 1;

        Serial.println(scha63x_cac_values.bzx, 6);
        Serial.println(scha63x_cac_values.bzy, 6);
        Serial.println(scha63x_cac_values.bzz, 6);
    }
    else
    {
        // Return error if test mode can not be activated
        return SCHA63X_ERR_TEST_MODE_ACTIVATION;
    }

    // HW Reset to get DUE asic out from test mode (reset via SPI).
    SPI_ASIC_DUE(SPI_FRAME_WRITE_REG_BANK_0); // Return to bank 0 to make SPI reset command available.
    SPI_ASIC_DUE(SPI_FRAME_WRITE_RESET);      // Reset DUE after reading cross-axis registers

    // Start UNO & DUE
    Wait_ms(25); // Wait 25ms for the non-volatile memory (NVM) Read

    // DUE asic initial startup
    SPI_ASIC_UNO(SPI_FRAME_WRITE_OP_MODE_NORMAL); // Set UNO operation mode on
    SPI_ASIC_DUE(SPI_FRAME_WRITE_OP_MODE_NORMAL); // Set DUE operation mode on twice
    SPI_ASIC_DUE(SPI_FRAME_WRITE_OP_MODE_NORMAL);
    Wait_ms(70); // Wait minimum 70ms (includes UNO 50ms 'SPI accessible' wait)

    SPI_ASIC_UNO(SPI_FRAME_WRITE_FILTER_RATE); // Select UNO 46Hz filter for RATE
    SPI_ASIC_UNO(SPI_FRAME_WRITE_FILTER_ACC);  // Select UNO 46Hz filter for ACC

    // Restart DUE
    SPI_ASIC_DUE(SPI_FRAME_WRITE_RESET); // Reset DUE again
    Wait_ms(25);                         // Wait 25 ms for the NVM read

    SPI_ASIC_DUE(SPI_FRAME_WRITE_OP_MODE_NORMAL); // Set DUE operation mode
    SPI_ASIC_DUE(SPI_FRAME_WRITE_OP_MODE_NORMAL); // DUE operation mode must be set twice

    Wait_ms(1);                                     // Wait 1 ms for SPI to be accesible
    SPI_ASIC_DUE(SPI_FRAME_WRITE_FILTER_RATE); // Select DUE 46Hz filter for RATE

    for (attempt = 0; attempt < num_attempts; attempt++)
    {
        // Wait FILTER_STARTUP_WAIT ms (Gyro and ACC start up)
        Wait_ms(FILTER_STARTUP_WAIT);

        // Set EOI=1 (End of initialization command)
        SPI_ASIC_UNO(SPI_FRAME_WRITE_EOI_BIT); // Set EOI bit for UNO
        SPI_ASIC_DUE(SPI_FRAME_WRITE_EOI_BIT); // Set EOI bit for DUE

        // Check initialization status from the summary status registers
        status_UNO = scha63x_check_init_uno();
        status_DUE = scha63x_check_init_due();

        // Check if restart is needed
        if ((status_UNO == false || status_DUE == false) && attempt < (num_attempts - 1))
        {

            SPI_ASIC_UNO(SPI_FRAME_WRITE_RESET);
            SPI_ASIC_DUE(SPI_FRAME_WRITE_RESET);

            Wait_ms(25);                                  // Wait 25 ms for the NVM read
            SPI_ASIC_UNO(SPI_FRAME_WRITE_OP_MODE_NORMAL); // Set UNO operation mode on
            SPI_ASIC_DUE(SPI_FRAME_WRITE_OP_MODE_NORMAL); // Set DUE operation mode on twice
            SPI_ASIC_DUE(SPI_FRAME_WRITE_OP_MODE_NORMAL);
            Wait_ms(50);                                    // Wait 50ms before communicating with UNO
            SPI_ASIC_UNO(SPI_FRAME_WRITE_FILTER_RATE); // Select UNO 46Hz filter for RATE
            SPI_ASIC_UNO(SPI_FRAME_WRITE_FILTER_ACC);  // Select UNO 46Hz filter for ACC
            SPI_ASIC_DUE(SPI_FRAME_WRITE_FILTER_RATE); // Select DUE 46Hz filter for RATE
            Wait_ms(45);                                    // Adjust restart duration to 500 ms
        }
        else
        {
            break;
        }
    }

    if (status_DUE == false || status_UNO == false)
    {
        return SCHA63X_ERR_RS_STATUS_NOK;
    }

    return SCHA63X_OK;
}


/*!
    \brief CAC struct getter
    \return pointer to a struct, check defs.h
*/
scha63x_cacv *get_cacv_ptr(void)
{
    return &scha63x_cac_values;
}

/*!
    \brief Check initialization success of SCHA63X sensor DUE asic

    \return true (success) or false (failure)
*/
static bool scha63x_check_init_due(void)
{
    uint32_t resp;
    bool due_ok;

    // Read summary status two times (first time may show incorrectly FAIL after start-up)
    SPI_ASIC_DUE(SPI_FRAME_READ_SUMMARY_STATUS);
    SPI_ASIC_DUE(SPI_FRAME_READ_SUMMARY_STATUS);
    Wait_ms(3);
    resp = SPI_ASIC_DUE(SPI_FRAME_READ_SUMMARY_STATUS);
    due_ok = SPI_DATA_CHECK_RS_ERROR(resp) == true ? false : true;

    return due_ok;
}

/*!
    \brief Check initialization success of SCHA63X sensor UNO asic

    \return true (success) or false (failure)
*/
static bool scha63x_check_init_uno(void)
{
    uint32_t resp;
    bool uno_ok;

    // Read summary status two times (first time may show incorrectly FAIL after start-up)
    SPI_ASIC_UNO(SPI_FRAME_READ_SUMMARY_STATUS);
    SPI_ASIC_UNO(SPI_FRAME_READ_SUMMARY_STATUS);
    Wait_ms(3);
    resp = SPI_ASIC_UNO(SPI_FRAME_READ_SUMMARY_STATUS);
    uno_ok = SPI_DATA_CHECK_RS_ERROR(resp) == true ? false : true;

    return uno_ok;
}




// Read sensor data

/*!
    \brief Read acceleration, rate and temperature data from sensor. 

    \param data pointer to "raw" data from sensor
*/
void scha63x_read_data(scha63x_raw_data *data)
{
    SPI_ASIC_DUE(SPI_FRAME_READ_GYRO_Y);                       // gyro y
    uint32_t gyro_y_lsb = SPI_ASIC_DUE(SPI_FRAME_READ_GYRO_Z); // gyro z
    uint32_t gyro_z_lsb = SPI_ASIC_DUE(SPI_FRAME_READ_TEMP);   // temperature
    uint32_t temp_due_lsb = SPI_ASIC_DUE(SPI_FRAME_READ_TEMP);

    SPI_ASIC_UNO(SPI_FRAME_READ_GYRO_X);                      // gyro x
    uint32_t gyro_x_lsb = SPI_ASIC_UNO(SPI_FRAME_READ_ACC_X); // acc x
    uint32_t acc_x_lsb = SPI_ASIC_UNO(SPI_FRAME_READ_ACC_Y);  // acc y
    uint32_t acc_y_lsb = SPI_ASIC_UNO(SPI_FRAME_READ_ACC_Z);  // acc z
    uint32_t acc_z_lsb = SPI_ASIC_UNO(SPI_FRAME_READ_TEMP);   // temperature
    uint32_t temp_uno_lsb = SPI_ASIC_UNO(SPI_FRAME_READ_TEMP);

    // Get possible errors
    data->rs_error_due = scha63x_check_rs_error_3(gyro_y_lsb, gyro_z_lsb, temp_due_lsb);
    data->rs_error_uno = scha63x_check_rs_error_5(gyro_x_lsb, acc_x_lsb, acc_y_lsb, acc_z_lsb, temp_uno_lsb);

    // Parse MISO data to structure
    data->acc_x_lsb = SPI_DATA_INT16(acc_x_lsb);
    data->acc_y_lsb = SPI_DATA_INT16(acc_y_lsb);
    data->acc_z_lsb = SPI_DATA_INT16(acc_z_lsb);
    data->gyro_x_lsb = SPI_DATA_INT16(gyro_x_lsb);
    data->gyro_y_lsb = SPI_DATA_INT16(gyro_y_lsb);
    data->gyro_z_lsb = SPI_DATA_INT16(gyro_z_lsb);
    data->temp_due_lsb = SPI_DATA_INT16(temp_due_lsb);
    data->temp_uno_lsb = SPI_DATA_INT16(temp_uno_lsb);
}



// Sensor status and error checks

/*!
    \brief Check if MISO frames have RS error bits set

    \param data pointer to 32-bit MISO frames from sensor
    \param size number of frames to check
    \return true (RS error bits set), false (no RS error)

*/
static bool scha63x_check_rs_error(uint32_t *data, int size)
{
    for (int i = 0; i < size; i++)
    {
        uint32_t value = data[i];
        if (SPI_DATA_CHECK_RS_ERROR(value))
        {
            return true;
        }
    }

    return false;
}

/*!
    \brief Check if any of three MISO frames have RS error bits set

    \param miso_1 ...miso_3 - MISO words 1...3
    \return true (RS error bits set), false (no RS error)
*/
static bool scha63x_check_rs_error_3(uint32_t miso_1, uint32_t miso_2, uint32_t miso_3)
{
    uint32_t miso_words[] = {miso_1, miso_2, miso_3};
    return scha63x_check_rs_error(miso_words, (sizeof(miso_words) / sizeof(uint32_t)));
}

/*!
    \brief Check if any of five MISO frames have RS error bits set

    \param miso_1 .. miso_5 - MISO words 1 .. 5
    \return true (RS error bits set), false (no RS error)
*/
static bool scha63x_check_rs_error_5(uint32_t miso_1, uint32_t miso_2, uint32_t miso_3,
                                     uint32_t miso_4, uint32_t miso_5)
{
    uint32_t miso_words[] = {miso_1, miso_2, miso_3, miso_4, miso_5};
    return scha63x_check_rs_error(miso_words, (sizeof(miso_words) / sizeof(uint32_t)));
}

/*!
    \brief Read sensor status from UNO ASIC

    \param status pointer to sensor status structure where the data is read to
*/
void scha63x_read_sensor_status_uno(scha63x_sensor_status *status)
{

    SPI_ASIC_UNO(SPI_FRAME_READ_SUMMARY_STATUS);
    uint32_t sum_stat = SPI_ASIC_UNO(SPI_FRAME_READ_RATE_STATUS_1);
    uint32_t rat_stat = SPI_ASIC_UNO(SPI_FRAME_READ_ACC_STATUS_1);
    uint32_t acc_stat = SPI_ASIC_UNO(SPI_FRAME_READ_COMMON_STATUS_1);
    uint32_t com_stat_1 = SPI_ASIC_UNO(SPI_FRAME_READ_COMMON_STATUS_2);
    uint32_t com_stat_2 = SPI_ASIC_UNO(SPI_FRAME_READ_COMMON_STATUS_2);

    status->summary_status = SPI_DATA_UINT16(sum_stat);
    status->rate_status1 = SPI_DATA_UINT16(rat_stat);
    status->acc_status1 = SPI_DATA_UINT16(acc_stat);
    status->common_status1 = SPI_DATA_UINT16(com_stat_1);
    status->common_status2 = SPI_DATA_UINT16(com_stat_2);
}

/*!
    \brief Read sensor status from DUE ASIC

    \param status pointer to sensor status structure where the data is read to
*/
void scha63x_read_sensor_status_due(scha63x_sensor_status *status)
{

    SPI_ASIC_DUE(SPI_FRAME_READ_SUMMARY_STATUS);
    uint32_t sum_stat = SPI_ASIC_DUE(SPI_FRAME_READ_RATE_STATUS_1);
    uint32_t rat_stat_1 = SPI_ASIC_DUE(SPI_FRAME_READ_RATE_STATUS_2);
    uint32_t rat_stat_2 = SPI_ASIC_DUE(SPI_FRAME_READ_COMMON_STATUS_1);
    uint32_t com_stat_1 = SPI_ASIC_DUE(SPI_FRAME_READ_COMMON_STATUS_2);
    uint32_t com_stat_2 = SPI_ASIC_DUE(SPI_FRAME_READ_COMMON_STATUS_2);

    status->summary_status = SPI_DATA_UINT16(sum_stat);
    status->rate_status1 = SPI_DATA_UINT16(rat_stat_1);
    status->rate_status2 = SPI_DATA_UINT16(rat_stat_2);
    status->common_status1 = SPI_DATA_UINT16(com_stat_1);
    status->common_status2 = SPI_DATA_UINT16(com_stat_2);
}
