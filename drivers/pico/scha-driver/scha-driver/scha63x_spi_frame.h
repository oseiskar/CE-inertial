#ifndef SCHA63X_SPI_FRAME_H
#define SCHA63X_SPI_FRAME_H

/*!
    @file scha63x_spi_frame.h
    @brief SPI frames for the sensor
*/

#include <stdint.h>
#include <stdbool.h>

#include "defs.h" 


/*! \brief Macro for bitshifting */
#define shift(frame, nbit) ((frame) = (frame << (nbit)))

// Pre-calculated SPI frames for various operations

///@{
/*! \brief Read RATE and ACC values from sensor */
#define SPI_FRAME_READ_GYRO_X 0x040000F7
#define SPI_FRAME_READ_GYRO_Y 0x0C0000FB
#define SPI_FRAME_READ_GYRO_Z 0x040000F7
#define SPI_FRAME_READ_ACC_X 0x100000E9
#define SPI_FRAME_READ_ACC_Y 0x140000EF
#define SPI_FRAME_READ_ACC_Z 0x180000E5
#define SPI_FRAME_READ_TEMP 0x1C0000E3

#define SPI_FRAME_READ_SUMMARY_STATUS 0x380000D5
#define SPI_FRAME_READ_RATE_STATUS_1 0x40000091
#define SPI_FRAME_READ_RATE_STATUS_2 0x44000097
#define SPI_FRAME_READ_COMMON_STATUS_1 0x50000089
#define SPI_FRAME_READ_COMMON_STATUS_2 0x5400008F
#define SPI_FRAME_READ_ACC_STATUS_1 0x4800009D
///@}

///@{
/*! \brief Reset and validation frames */
#define SPI_FRAME_WRITE_RESET 0xE000017C
#define SPI_FRAME_WRITE_REG_BANK_0 0xFC000073 // Select register bank 0
#define SPI_FRAME_READ_TRC_0 0x740000BF       // Traceability 0 register 1Dh
#define SPI_FRAME_READ_TRC_1 0x780000B5       // Traceability 1 register 1Eh
#define SPI_FRAME_READ_TRC_2 0x700000B9       // Traceability 2 Register 1Ch
///@}

///@{
/*! \brief Frames needed for test mode activation. Mode register address: 19h */
#define SPI_FRAME_READ_MODE 0x640000A7
#define SPI_FRAME_WRITE_MODE_ASM_010 0xE40010AA // Unlock_ASM[2:0] = 010 
#define SPI_FRAME_WRITE_MODE_ASM_001 0xE400088F // Unlock_ASM[2:0] = 001
#define SPI_FRAME_WRITE_MODE_ASM_100 0xE40020E0 // Unlock_ASM[2:0] = 100
///@}


#define SPI_REGISTER_COMMON_STATUS_1 0x14
#define SPI_REGISTER_COMMON_STATUS_2 0x15
#define SPI_REGISTER_SYS_TEST 0x17
#define SPI_REGISTER_SUMMARY_STATUS 0x0E

// Filter configuration

///@{
/*! \brief Filter configuration frames */
#define SPI_FRAME_WRITE_OP_MODE_NORMAL 0xE4000067 // Set normal operating mode (register 19h)
#define SPI_FRAME_WRITE_EOI_BIT 0xE000025B
///@}

// for testing
#define SPI_FRAME_WRITE_FILTER_46HZ_RATE 0xD812129E // Set 46 Hz filter for rate (register 16h)
#define SPI_FRAME_WRITE_FILTER_46HZ_ACC 0xE8022248  // Set 46 Hz filter for acceleration (register 1Ah)

/*! \brief gyro (register 16h) */
#define GYRO_REG 0b10110 
/*! \brief accelerometer (register 1Ah) */
#define ACC_REG  0b11010 

uint32_t generate_acc_frame(_acc_conf conf);
uint32_t generate_uno_gyro_frame(_gyro_conf conf);
uint32_t generate_due_gyro_frame(_gyro_conf conf);

uint8_t CalculateCRC(uint32_t Data);
uint8_t CRC8(uint8_t BitValue, uint8_t CRC);

uint32_t generate_spi_frame(uint8_t is_write, uint8_t reg, uint16_t data);
uint32_t generate_sys_test_write(uint16_t test_data);

#define SPI_FRAME_READ_SYS_TEST generate_spi_frame(0, SPI_REGISTER_SYS_TEST, 0)

#endif