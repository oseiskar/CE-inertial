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

    struct scha_filters _f = { 0b000, 0b001, 0b010, 0b011, 0b100 };

    struct _acc_conf acc = {_f.FILTER_46HZ, _f.FILTER_46HZ, _f.FILTER_46HZ};
    struct _gyro_conf gyro = {_f.FILTER_46HZ, _f.FILTER_46HZ, _f.FILTER_46HZ, _f.FILTER_46HZ};
    struct scha63x_sensor_config sensor_config = {acc, gyro};

    status = initialize_sensor(serial_num, &sensor_config);    
    if (status != SCHA63X_OK) {
        printf("Init failed\n");
        printf(serial_num);

        printf("Init failed\n");
        while (true);
    }
    printf(serial_num);

}