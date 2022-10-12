#include "scha63x_spi.h"
#include <stdio.h>

/*!
    @file scha63x_spi.cpp
    @brief SPI communication between Murata and !"#!"#!"#"
*/

/*! \brief SPI SCK clock signal rate */
const uint32_t transfer_rate = 100 * 1000; 
/*! \brief SPI settings */
// EDIT
// const SPISettings spi_set(transfer_rate, MSBFIRST, SPI_MODE0);

///@{
/*!
    \brief 
*/
#define PIN_MISO    8
#define PIN_SCK     10
#define PIN_MOSI    11
#define PIN_CS_UNO  5
#define PIN_CS_DUE  9

#define SPI_PORT spi1
///@}

static inline void cs_select(int pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(pin, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(int pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(pin, 1);
    asm volatile("nop \n nop \n nop");
}

/*!
    \brief Initialize SPI
*/
void spi_initialize(void)
{
    printf("Hello! Reading raw data from registers via SPI...\n");

    spi_cpha_t a = {0};
    spi_cpol_t b = {1}; 
    spi_order_t c = {1}; 

    const int actual_rate = spi_init(SPI_PORT, transfer_rate);
    printf("spi_init return: %d baud\n", actual_rate);

    spi_set_format(SPI_PORT, 8, a, b, c);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Make the SPI pins available to picotool
    bi_decl(bi_3pins_with_func(PIN_MISO, PIN_MOSI, PIN_SCK, GPIO_FUNC_SPI));

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PIN_CS_DUE);
    gpio_init(PIN_CS_UNO);
    gpio_set_dir(PIN_CS_DUE, GPIO_OUT);
    gpio_set_dir(PIN_CS_UNO, GPIO_OUT);
    gpio_put(PIN_CS_DUE, 1);
    gpio_put(PIN_CS_UNO, 1);

    // Make the CS pin available to picotool
    bi_decl(bi_1pin_with_name(PIN_CS_DUE, "SPI CS0"));
    bi_decl(bi_1pin_with_name(PIN_CS_UNO, "SPI CS1"));
}

/*!
    \brief SPI communication with ASICs

    Send new and receive previous SPI frame. Called from wrapper functions below. 
    SCHA63X data frame length is 32 bits and the data must be sent out MSB first.
    MSBFIRST has been set on SPI_Initialize(), so no need to swap order of the bits. 
    Arduino SPI library does not support sending 32 bits -> 32 bit frame is divided 
    into four frames of 8 bits (SPI library's transfer16 is slower)
    // buffer allocation not really beneficial when sending 32 bits
    // static spi_transaction_t tr;
  
    \param data frame to send
    \param asic_number devices SPI CS pin number
*/
static uint32_t SPI_ASIC( uint32_t data, int asic_number)
{
    uint8_t b1 = ((data >> 24) & 0xff);
    uint8_t b2 = ((data >> 16) & 0xff);
    uint8_t b3 = ((data >> 8) & 0xff);
    uint8_t b4 = ((data) & 0xff);
    uint8_t ui1, ui2, ui3, ui4;

    cs_select(asic_number);
    spi_write_read_blocking(SPI_PORT, &b1, &ui1, 1); 
    spi_write_read_blocking(SPI_PORT, &b2, &ui2, 1); 
    spi_write_read_blocking(SPI_PORT, &b3, &ui3, 1); 
    spi_write_read_blocking(SPI_PORT, &b4, &ui4, 1); 
    cs_deselect(asic_number);

    uint32_t output_spi = ( (uint32_t)((ui1 & 0xFF) << 24) | (uint32_t)((ui2 & 0xFF) << 16) | (uint32_t)((ui3 & 0xFF) << 8) | (uint32_t)(ui4 & 0xFF) ) ;
    
    // printf("%d\n", ui1);
    // printf("%d\n", ui2);
    // printf("%d\n", ui3);
    // printf("%d\n", ui4);
    printf("SPI_ASIC debug: %08x -> %08x\n", data, output_spi);

    return output_spi;
}


/*!
    \brief Wrapper for SPI_ASIC, accesses UNO
*/
uint32_t SPI_ASIC_UNO(uint32_t dout)
{
  return SPI_ASIC(dout, PIN_CS_UNO);
}

/*!
    \brief Wrapper for SPI_ASIC, accesses DUE
*/
uint32_t SPI_ASIC_DUE(uint32_t dout)
{
  return SPI_ASIC(dout, PIN_CS_DUE);
}
