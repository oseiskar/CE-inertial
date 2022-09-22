#ifndef scha63x_spi_H
#define scha63x_spi_H

#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

/*!
    @file scha63x_spi.h
    @brief SPI communication between Murata and MCU
*/
#ifdef __cplusplus 
 extern "C" {   
#endif

void spi_initialize(void);
uint32_t SPI_ASIC_DUE(uint32_t dout);
uint32_t SPI_ASIC_UNO(uint32_t dout);

#ifdef __cplusplus
}
#endif 

#endif
