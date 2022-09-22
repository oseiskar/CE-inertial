/*!
    @file config.h
    @brief Configuration file for project
    
    Used sensor variant and other settings are defined here 
    with precompiler settings.
*/

#ifndef CONFIG_H
#define CONFIG_H


///@{
/*!
    \brief Sensor variant and sensitivity settings
*/

// Used SCHA634 variant
#define SCHA63X_X01   // Use SCHA634-D01 or SCHA63T-K01
//#define SCHA634_D02 // Use SCHA634-D02
//#define SCHA63X_X03 // Use SCHA634-D03 or SCHA63T-K03

#define SENSITIVITY_ACC 4905

#ifdef SCHA63X_X01
#define SENSITIVITY_GYRO_X 160
#define SENSITIVITY_GYRO_Y 160
#define SENSITIVITY_GYRO_Z 160
#elif defined SCHA634_D02
#define SENSITIVITY_GYRO_X 160
#define SENSITIVITY_GYRO_Y 160
#define SENSITIVITY_GYRO_Z 80
#elif defined SCHA63X_X03
#define SENSITIVITY_GYRO_X 80
#define SENSITIVITY_GYRO_Y 80
#define SENSITIVITY_GYRO_Z 80
#endif

#define SENSOR_TYPE 1

///@}

///@{
/*!
    \brief IMU settings
*/

#define IMU_SAMPLING_RATE 500 // sampling rate, n.b limitations with transfer speed
#define BUFFER_SIZE 2 // IMU buffer size, n.b limitations with transfer speed
#define STRUCT_SIZE 26  // 64 + 8 * 16 + 4 * 4

///@}


#endif // CONFIG_H