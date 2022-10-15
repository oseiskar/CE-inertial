#ifndef SCHA63X_DRIVER_H
#define SCHA63X_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#include "defs.h" 

/*!
    @file scha63x_driver.h
    @brief Driver for SCHA63X sensor
*/

// Negative values = errors, positive values = warnings
// NOTE: Error code always overrides warning, when returning from function
#define SCHA63X_OK                         0
#define SCHA63X_ERR_TEST_MODE_ACTIVATION   -1 // error, Could not activate test mode during init
#define SCHA63X_ERR_RS_STATUS_NOK          -2 // error, RS status not OK after all init steps
#define SCHA63X_ERR_SYS_TEST               -3 // sys_test register r/w test failed
#define SCHA63X_ERR_CRC_FAIL               -4

int  initialize_sensor(char *serial_num, scha63x_sensor_config *config);

scha63x_cacv* get_cacv_ptr(void);

void scha63x_read_data(scha63x_raw_data *data);
void scha63x_read_sensor_status_uno(scha63x_sensor_status *status);
void scha63x_read_sensor_status_due(scha63x_sensor_status *status);

#endif // #ifndef SCHA63X_H
