#include "scha63x_spi_frame.h"

/*!
    @file scha63x_spi_frame.cpp
    @brief SPI frames for the sensor
*/

// SPI frame generation

/*!
    \brief Generate SPI frames for gyro and accelerometer filter

    \param config pointer to sensor filter configuration values in a struct
*/
void generate_filter_frames(scha63x_sensor_config *config)
{
    // startup wait of 525ms for 46Hz filters and above 
    if (config->acc_filter.Ax > 2) {
        #undef FILTER_STARTUP_WAIT
        #define FILTER_STARTUP_WAIT 525
    }

    #undef SPI_FRAME_WRITE_FILTER_RATE
    #define SPI_FRAME_WRITE_FILTER_RATE generate_gyro_frame(config->gyro_filter);

    #undef SPI_FRAME_WRITE_FILTER_ACC
    #define SPI_FRAME_WRITE_FILTER_ACC generate_acc_frame(config->acc_filter);   
}

/*!
    \brief Generate valid header for frame

    \param rw read or write frame
    \param reg operation register 
    \param rs return status
    \return 8-bit header
*/
uint8_t generate_frame_header(uint8_t rw, uint8_t reg, uint8_t rs){
    
    uint8_t header = 0; header |= rw;  // OP 5 w(1) / r(0)
    shift(header, 5); header |= reg;   // OP 4:0 register
    shift(header, 2); header |= rs;    // RS 1:0

    return header;
}

/*!
    \brief Generate SPI frame for RATE
        
    Gyroscope RATE (documentation 6.2.8)

    \param FILTER struct of filter values
    \return generated 32-bit SPI frame with checksum
*/
uint32_t generate_gyro_frame(_gyro_conf FILTER)
{
    uint32_t frame = 0;
    frame |= generate_frame_header(1, GYRO_REG, 0);

    // data 
    shift(frame, 2); frame |= 0b00;           // D14-D15 -> TODO
    shift(frame, 3); frame |= FILTER.Rz2_Rx2; // D11-D13 -> Rz2/Rx2 
    shift(frame, 3); frame |= FILTER.Rz_Rx;   // D8-D10 -> Rz/Rx
    shift(frame, 2); frame |= 0b00;           // D6-D7 -> TODO
    shift(frame, 3); frame |= FILTER.Ry;      // D3-D5 -> Ry 
    shift(frame, 3); frame |= FILTER.Ry2;     // D0-D2 -> Ry2
    
    // checksum 
    shift(frame, 8); frame |= CalculateCRC(frame);

    return frame;
}

/*!
    \brief Generate SPI frame for ACC
        
    Accelerometer ACC (documentation 6.2.12)

    \param FILTER struct of filter values
    \return generated 32-bit SPI frame with checksum
*/
uint32_t generate_acc_frame(_acc_conf FILTER)
{
    uint32_t frame = 0;
    frame |= generate_frame_header(1, ACC_REG, 0);

    // data 
    shift(frame, 5); frame |= 0b00000;    // D12-D15
    shift(frame, 3); frame |= FILTER.Ax;  // D8-D10 Ax 
    shift(frame, 1); frame |= 0;          // D7
    shift(frame, 3); frame |= FILTER.Ay;  // D4-D6 Ay
    shift(frame, 1); frame |= 0;          // D3
    shift(frame, 3); frame |= FILTER.Az;  // D0-D2 Az

    // checksum 
    shift(frame, 8); frame |= CalculateCRC(frame);

    return frame;
}

/*!
    \brief Calculate CRC 
    
    Calculated for 24 MSB's of the 32 bit dword (8 LSB's are 
    the CRC field and are not included in CRC calculation). 
    Documentation 1.5.6

    \param Data 32-bit dataframe
    \return CRC of a dataframe

*/ 
uint8_t CalculateCRC(uint32_t Data)
{
    uint8_t BitIndex;
    uint8_t BitValue;
    uint8_t CRC;
    CRC = 0xFF;
    for (BitIndex = 31; BitIndex > 7; BitIndex--)
    {
        BitValue = (uint8_t)((Data >> BitIndex) & 0x01);
        CRC = CRC8(BitValue, CRC);
    }
    CRC = (uint8_t)~CRC;
    return CRC;
}

/*!
    \brief Checksum calculation for 8 bits

    Check documentation 1.5.6 for CRC formula

    \param BitValue 
    \param CRC
*/
uint8_t CRC8(uint8_t BitValue, uint8_t CRC)
{
    uint8_t Temp;
    Temp = (uint8_t)(CRC & 0x80);
    if (BitValue == 0x01)
    {
        Temp ^= 0x80;
    }
    CRC <<= 1;
    if (Temp > 0)
    {
        CRC ^= 0x1D;
    }
    return CRC;
}

