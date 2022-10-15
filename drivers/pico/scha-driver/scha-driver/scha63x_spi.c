#include "scha63x_spi.h"
#include <stdio.h>

/*!
    @file scha63x_spi.cpp
    @brief SPI communication between Murata and !"#!"#!"#"
*/

/*! \brief SPI SCK clock signal rate. Minimum 0.1MhZ, max 10MHz according to spec */
const uint32_t transfer_rate = 1 * 1000 * 1000; // 1 MHz
/*! \brief SPI settings */
// EDIT
// const SPISettings spi_set(transfer_rate, MSBFIRST, SPI_MODE0);

///@{
/*!
    \brief 
*/
#define PIN_SCK     10
#define PIN_MOSI    11
#define PIN_MISO    12
#define PIN_CS_DUE  13
#define PIN_CS_UNO  9
#define PIN_RES_DUE 6
#define PIN_RES_UNO 8

#define SPI_PORT spi1
///@}

#define ENABLE_SPI_DEBUG
#ifdef ENABLE_SPI_DEBUG
#include "scha63x_spi_frame.h"
#endif

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
    spi_cpha_t a = {0};
    spi_cpol_t b = {1}; 
    spi_order_t c = {1}; 

    const int actual_rate = spi_init(SPI_PORT, transfer_rate);
#ifdef ENABLE_SPI_DEBUG
    printf("spi_init return: %d baud\n", actual_rate);
#endif

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

    // Make the CS pins available to picotool
    bi_decl(bi_1pin_with_name(PIN_CS_DUE, "SPI CS0"));
    bi_decl(bi_1pin_with_name(PIN_CS_UNO, "SPI CS1"));

    // make sure the "ext reset pins" are set to high
    gpio_init(PIN_RES_DUE);
    gpio_init(PIN_RES_UNO);
    gpio_set_dir(PIN_RES_DUE, GPIO_OUT);
    gpio_set_dir(PIN_RES_UNO, GPIO_OUT);
    gpio_put(PIN_RES_DUE, 1);
    gpio_put(PIN_RES_UNO, 1);

    bi_decl(bi_1pin_with_name(PIN_RES_DUE, "0/DUE EXT RESET"));
    bi_decl(bi_1pin_with_name(PIN_RES_UNO, "1/UNO EXT RESET"));
}

void reset_ext_gpio(bool reset_uno, bool reset_due) {
    if (reset_uno) gpio_put(PIN_RES_UNO, 0);
    if (reset_due) gpio_put(PIN_RES_DUE, 0);
    sleep_ms(1);
    if (reset_uno) gpio_put(PIN_RES_UNO, 1);
    if (reset_due) gpio_put(PIN_RES_DUE, 1);
}

#ifdef ENABLE_SPI_DEBUG
static void debug_spi_frame(uint32_t frame) {
    uint8_t crc = frame & 0xff;
    uint8_t expectedCrc = CalculateCRC(frame);

    uint8_t rs = (frame >> 24) & 0x3;
    uint16_t data = (frame >> 8) & 0xffff;
    uint8_t op = (frame >> 26) & 0xff;
    uint8_t rw_bit = (op >> 5);
    uint8_t addr = op & 31;

    char *rs_str = "";
    if (rs == 0) rs_str = "IN";
    else if (rs == 1) rs_str = "OK";
    else if (rs == 2) rs_str = "ST";
    else if (rs == 3) rs_str = "ER";

    char *reg_str = "??????";
    switch (addr) {
#define X(x, y) case x: reg_str = y; break;
    X(0x00, "0_none")
    X(0x01, "RxOrRz")
    X(0x03, "Rate_Y")
    X(0x04, "Acc_X_")
    X(0x05, "Acc_Y_")
    X(0x06, "Acc_Z_")
    X(0x07, "TEMP__")
    X(0x0b, "RZ2RX2")
    X(0x0d, "RY2___")
    X(SPI_REGISTER_SUMMARY_STATUS, "S_stat")
    X(0x0f, "SCtrl_")
    X(SPI_REGISTER_COMMON_STATUS_1, "CStat1")
    X(SPI_REGISTER_COMMON_STATUS_2, "CStat2")
    X(0x16, "G_FILT")
    X(SPI_REGISTER_SYS_TEST, "SysTst")
    X(0x18, "ResCTR")
    X(0x19, "OpMode")
    X(0x1a, "A_FILT")
    X(0x1c, "T_ID2_")
    X(0x1d, "T_ID0_")
    X(0x1e, "T_ID1_")
    X(0x1f, "SelBnk")
#undef X
    default: break;
    }

    printf("op:%s|%s(%02x),rs%s,data:%04x,crc:", rw_bit ? "W" : "R",reg_str, addr, rs_str, data);
    if (crc == expectedCrc) printf("OK");
    else printf("FAIL[%02x vs %02x]", crc, expectedCrc);
}
#endif

/*!
    \brief SPI communication with ASICs

    Send new and receive previous SPI frame. Called from wrapper functions below. 
    SCHA63X data frame length is 32 bits and the data must be sent out MSB first.
    MSBFIRST has been set on SPI_Initialize(), so no need to swap order of the bits. 
  
    \param data frame to send
    \param asic_number devices SPI CS pin number
*/
static uint32_t SPI_ASIC( uint32_t data, int asic_number)
{
    uint8_t b[4], u[4];
    b[0] = ((data >> 24) & 0xff);
    b[1] = ((data >> 16) & 0xff);
    b[2] = ((data >> 8) & 0xff);
    b[3] = ((data) & 0xff);

    // sleep_ms(100);

    cs_select(asic_number);
    spi_write_read_blocking(SPI_PORT, b, u, 4);
    cs_deselect(asic_number);
    
    // sleep_ms(100);

    uint32_t output_spi = ( (uint32_t)((u[0] & 0xFF) << 24) | (uint32_t)((u[1] & 0xFF) << 16) | (uint32_t)((u[2] & 0xFF) << 8) | (uint32_t)(u[3] & 0xFF) ) ;
    
#ifdef ENABLE_SPI_DEBUG
    printf("SPI debug %s: %08x -> %08x: ", asic_number == PIN_CS_UNO ? "UNO" : "DUE", data, output_spi);
    debug_spi_frame(data);
    printf(" -> ");
    debug_spi_frame(output_spi);
    printf("\n");
#endif

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

uint32_t SPI_ASIC_SELECT(uint32_t dout, uint8_t is_uno) {
    if (is_uno) return SPI_ASIC_UNO(dout);
    else return SPI_ASIC_DUE(dout);
}
