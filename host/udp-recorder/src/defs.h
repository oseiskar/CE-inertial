#ifndef DEFS_H
#define DEFS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdint.h>
#include <stdbool.h>

/*!
    @file defs
    @brief struct defs used in program
*/



/* RUNTIME STRUCTS */

/*! 
    \brief Converted real data from raw data
    
    based on sensor sensitivity received in init, size 
    32 bytes / element
*/
typedef struct _scha63x_real_data {
    
    float acc_x;
    float acc_y;
    float acc_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float temp_due;
    float temp_uno;
        
} scha63x_real_data;

/*! 
    \brief Raw data values from sensor and triggerinfo 
    sent over UDP for computation and data storage
*/
typedef struct _scha63x_raw_data {

    int64_t timeStamp;
    
    int16_t acc_x_lsb;
    int16_t acc_y_lsb;
    int16_t acc_z_lsb;
    int16_t gyro_x_lsb;
    int16_t gyro_y_lsb;
    int16_t gyro_z_lsb;
    int16_t temp_due_lsb;
    int16_t temp_uno_lsb;    
        
    bool rs_error_due;
    bool rs_error_uno;

    bool cam_trigger;
    bool ubx_trigger;
    
} scha63x_raw_data;

/*! 
    \brief Cross axis compensation values 
*/
typedef struct _scha63x_cacv {
    
    // c-values
    float cxx; float cxy; float cxz;
    float cyx; float cyy; float cyz;
    float czx; float czy; float czz;
    
    // b-values
    float bxx; float bxy; float bxz;
    float byx; float byy; float byz;
    float bzx; float bzy; float bzz;
        
} scha63x_cacv;



/* STRUCTS USED IN INIT/SETUP */

/*!
    \brief UDP connection settings

*/
struct _udp
{
    int sock;
    sockaddr_in from;
    socklen_t fromlen;
};

/*!
    \brief Sensor configuration data

    if status is 0 then initialization has failed, 
    buffer size is 0 when this struct has been received 
    from Arduino. 
    the struct and trigger values are inserted from main program
    and struct will be sent back to Arduino. 
    Size of struct will be less than 100 bytes. 
*/
typedef struct _sensor_data {

    int status;
    char serial_num[14];

    // // No need to sed these due to Arduino just passing raw data forward
    // int sensivity_acc;
    // int sensivity_gyro_x;
    // int sensivity_gyro_y;
    // int sensivity_gyro_z;

    int buffer;
    int imu_trigger;
    int cam_trigger;   // trigger length defined in scha63x config.h

} sensor_data;


// Filter setup

/*! 
    \brief Sensor filter configuration for accelerometer
*/
typedef struct _acc_conf  {
    
	uint16_t Ax;
    uint16_t Ay;
    uint16_t Az;
            
} acc_conf;

/*! 
    \brief Sensor filter configuration for gyro
*/
typedef struct _gyro_conf {
    
	uint16_t Rz2_Rx2;
    uint16_t Rz_Rx;
    uint16_t Ry2;
    uint16_t Ry;
            
} gyro_conf;

/*! 
    \brief Sensor filter configurations for gyro 
    struct and acc struct
*/
typedef struct _scha63x_sensor_config {

    _acc_conf acc_filter;
	_gyro_conf gyro_filter;

} scha63x_sensor_config;


#endif