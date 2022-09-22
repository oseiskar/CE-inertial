#include <SPI.h>
#include "scha63x_spi.h"
#include "arduino_timers.h"


/*!
    @file scha63x_spi.cpp
    @brief SPI communication between Murata and Arduino
*/

/*! \brief SPI SCK clock signal rate */
const uint32_t transfer_rate = 10 * 1000 * 1000; 
/*! \brief SPI settings */
const SPISettings spi_set(transfer_rate, MSBFIRST, SPI_MODE0);

///@{
/*!
    \brief Arduino pin definition from config.h
*/
#define CSB_UNO  MURATA_ASIC_UNO_CS     // CSB UNO pin J3-1 
#define CSB_DUE  MURATA_ASIC_DUE_CS     // CSB DUE pin J2-3 
#define CSB_ETH  W5X00_ETHERNET_CS_PIN  // W5100 ETHERNET CSB
#define CSB_SD   W5X00_SD_CS_PIN        // W5100 SD READER CSB
#define CSB_MEGA 53                     // MEGA CSB
///@}


/*!
    \brief Initialize SPI pins and start SPI object
*/
void SPI_Initialize(void)
{
  pinMode( CSB_UNO,  OUTPUT );
  pinMode( CSB_DUE,  OUTPUT ); 
  pinMode( CSB_MEGA, OUTPUT );
  pinMode( CSB_ETH,  OUTPUT );
  pinMode( CSB_SD,   OUTPUT );

  digitalWrite( CSB_UNO, HIGH );
  digitalWrite( CSB_DUE, HIGH );
  digitalWrite( CSB_SD,  HIGH );
  digitalWrite( CSB_ETH, HIGH );

  SPI.begin();
}


/*!
    \brief SPI communication with ASICs

    Send new and receive previous SPI frame. Called from wrapper functions below. 
    SCHA63X data frame length is 32 bits and the data must be sent out MSB first.
    MSBFIRST has been set on SPI_Initialize(), so no need to swap order of the bits. 
    Arduino SPI library does not support sending 32 bits -> 32 bit frame is divided 
    into four frames of 8 bits (SPI library's transfer16 is slower)
  
    \param dout frame to send
    \param GPIO_Pin devices SPI CS pin number
*/
static uint32_t SPI_ASIC(uint32_t dout, uint8_t GPIO_Pin)
{
  uint32_t val = 0;
  uint8_t b1 = ((dout >> 24) & 0xff);
  uint8_t b2 = ((dout >> 16) & 0xff);
  uint8_t b3 = ((dout >> 8) & 0xff);
  uint8_t b4 = ((dout) & 0xff);

  SPI.beginTransaction(spi_set);
  digitalWrite(GPIO_Pin, LOW); // Set CSB active

  // Transfer data
  val = SPI.transfer(b1); val <<= 8;
  val |= SPI.transfer(b2); val <<= 8;
  val |= SPI.transfer(b3); val <<= 8;
  val |= SPI.transfer(b4);

  digitalWrite(GPIO_Pin, HIGH); // Set CSB inactive
  SPI.endTransaction();

  return val;
}


/*!
    \brief Wrapper for SPI_ASIC, accesses DUE
*/
uint32_t SPI_ASIC_DUE(uint32_t dout)
{
  return SPI_ASIC(dout, CSB_DUE);
}

/*!
    \brief Wrapper for SPI_ASIC, accesses UNO
*/
uint32_t SPI_ASIC_UNO(uint32_t dout)
{
  return SPI_ASIC(dout, CSB_UNO);
}
