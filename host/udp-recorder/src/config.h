/*!
    @file config.h
    @brief Configuration file for project
    
    Used sensor variant and other settings are defined here 
    with precompiler settings.
*/

#ifndef CONFIG_H
#define CONFIG_H


///@{
/*! \brief Networking settings */
#define port 5555
#define buffer_size 100
#define struct_size 26  // size of a single IMU data struct
///@}


///@{
/*! \brief Size of IMU buffer and struct */
#define imu_buffer_size 4
#define imu_trigger_rate 500
///@}


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

///@{
/*! \brief IMU filter settings */

// Accel
#define Acc_Ax 0b010
#define Acc_Ay 0b010
#define Acc_Az 0b010

// Gyro
#define Gyro_Rz2_Rx2 0b010
#define Gyro_Rz_Rx 0b010
#define Gyro_Ry2 0b010
#define Gyro_Ry 0b010

///@}


/*! \brief camera trigger rate */
#define cam_trigger_rate 30


///@{
/*! \brief Sensor variant and sensitivity settings */
// Used SCHA634 variant
#define SCHA63X_X01     // SCHA634-D01 or SCHA63T-K01
//#define SCHA634_D02   // SCHA634-D02
//#define SCHA63X_X03   // SCHA634-D03 or SCHA63T-K03

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
///@}

#endif