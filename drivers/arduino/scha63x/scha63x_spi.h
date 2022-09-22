#ifndef scha63x_spi_H
#define scha63x_spi_H

#include <stdint.h>
#include <stdbool.h>
#include <Arduino.h>
#include <config.h>

/*!
    @file scha63x_spi.h
    @brief SPI communication between Murata and Arduino
*/


#ifdef __cplusplus 
    extern "C" {   
#endif

void SPI_Initialize(void);
uint32_t SPI_ASIC_DUE(uint32_t dout);
uint32_t SPI_ASIC_UNO(uint32_t dout);

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus required for Arduino C++ compability


#endif
