/*!
    @file defs.h
    @brief Internal defs used by the program
*/

#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>
#include <stdbool.h>

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
    \brief Sensor status
*/
typedef struct _scha63x_sensor_status {
    
	uint16_t summary_status;
	uint16_t rate_status1;
    uint16_t rate_status2;
    uint16_t acc_status1;
    uint16_t common_status1;
    uint16_t common_status2;
            
} scha63x_sensor_status;

/*! 
    \brief Sensor filter configurations for gyro and acc
*/
struct filters {
    
	static const uint8_t FILTER_13HZ  = 0b000;
    static const uint8_t FILTER_20HZ  = 0b001;
    static const uint8_t FILTER_46HZ  = 0b010;
    static const uint8_t FILTER_200HZ = 0b011;
    static const uint8_t FILTER_300HZ = 0b100;
            
};

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