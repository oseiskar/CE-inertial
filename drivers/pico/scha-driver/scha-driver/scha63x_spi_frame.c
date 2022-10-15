#include "scha63x_spi_frame.h"

/*!
    @file scha63x_spi_frame.cpp
    @brief SPI frames for the sensor
*/

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
uint32_t generate_uno_gyro_frame(_gyro_conf filter)
{
    uint16_t data = 0;

    // simplified
    uint8_t NOMINAL_DYN_RANGE_BIT_RX = 0; // nominal, 1 = nominal x2
    uint8_t Rx2 = filter.filter;
    uint8_t Rx = filter.filter;
    
    // D15: reserved
    shift(data, 1); data |= NOMINAL_DYN_RANGE_BIT_RX; // D14
    shift(data, 3); data |= Rx2; // D11-D13
    shift(data, 3); data |= Rx;   // D8-D10
    shift(data, 8); // D0-D7: Reserved

    return generate_spi_frame(1, GYRO_REG, data);
}

uint32_t generate_due_gyro_frame(_gyro_conf filter)
{
    uint16_t data = 0;

    // simplified
    uint8_t NOMINAL_DYN_RANGE_BIT_RZ = 0; // nominal, 1 = nominal x2
    uint8_t NOMINAL_DYN_RANGE_BIT_RY = 0;
    uint8_t Rz2 = filter.filter;
    uint8_t Rz = filter.filter;
    uint8_t Ry2 = filter.filter;
    uint8_t Ry = filter.filter;

    shift(data, 1); data |= NOMINAL_DYN_RANGE_BIT_RZ; // D14
    shift(data, 3); data |= Rz2; // D11-D13
    shift(data, 3); data |= Rz;   // D8-D1
    shift(data, 1); // D7: reserved
    shift(data, 1); data |= NOMINAL_DYN_RANGE_BIT_RY; // D6
    shift(data, 3); data |= Ry2;      // D3-D5  
    shift(data, 3); data |= Ry;     // D0-D2

    return generate_spi_frame(1, GYRO_REG, data);
}

/*!
    \brief Generate SPI frame for ACC
        
    Accelerometer ACC (documentation 6.2.12)

    \param FILTER struct of filter values
    \return generated 32-bit SPI frame with checksum
*/
uint32_t generate_acc_frame(_acc_conf filter)
{
    uint16_t data = 0;

    uint8_t NOMINAL_DYN_RANGE_BIT_AX = 0; // nominal, 1 = nominal x4
    uint8_t NOMINAL_DYN_RANGE_BIT_AY = 0;
    uint8_t NOMINAL_DYN_RANGE_BIT_AZ = 0;
    uint8_t Ax = filter.filter;
    uint8_t Ay = filter.filter;
    uint8_t Az = filter.filter;

    // data 
    // D12-D15: Reserved
    shift(data, 1); data |= NOMINAL_DYN_RANGE_BIT_AX; // D11
    shift(data, 3); data |= Ax; // D08-D10
    shift(data, 1); data |= NOMINAL_DYN_RANGE_BIT_AY; // D7
    shift(data, 3); data |= Ay;      // D4-D6  
    shift(data, 1); data |= NOMINAL_DYN_RANGE_BIT_AZ; // D3
    shift(data, 3); data |= Az;      // D0-D2  

    return generate_spi_frame(1, ACC_REG, data);
}

uint32_t generate_spi_frame(uint8_t is_write, uint8_t reg, uint16_t data) {
    uint32_t dout = ((uint32_t)generate_frame_header(is_write, reg, 0) << 24) |
        ((uint32_t)data << 8);
    dout |= CalculateCRC(dout);
    return dout;
}

uint32_t generate_sys_test_write(uint16_t data) {
    return generate_spi_frame(1, SPI_REGISTER_SYS_TEST, data);
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

