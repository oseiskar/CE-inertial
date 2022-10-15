#include "scha63x-runner.h"

#include "scha63x_spi.h"
#include "scha63x_driver.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"

void scha63x_runner(void)
{
    printf("bruh\n");
    // spi init
    spi_initialize();

    printf("Log serial number\n");

    // // sensor init
    char serial_num[14];
    int status;

    // struct scha_filters _f = { 0b000, 0b001, 0b010, 0b011, 0b100 };

    uint32_t acc_filter = FILTER_300HZ;
    uint32_t gyro_filter = FILTER_300HZ;
    struct _acc_conf acc = { acc_filter };
    struct _gyro_conf gyro = { gyro_filter };
    struct scha63x_sensor_config sensor_config = {acc, gyro};

    status = initialize_sensor(serial_num, &sensor_config);    
    if (status != SCHA63X_OK) {
        printf("Init failed\n");
        // return;
    }

    printf("serial number: %s\n", serial_num);
    while(true) {
        scha63x_raw_data data;
        scha63x_read_data(&data);
        printf("gyro: %d,%d,%d\tacc: %d,%d,%d, temp: %d\n",
            data.gyro_x_lsb,
            data.gyro_y_lsb,
            data.gyro_z_lsb,
            data.acc_x_lsb,
            data.acc_y_lsb,
            data.acc_z_lsb,
            data.temp_due_lsb);
        sleep_ms(10);
        break;
    }

}
