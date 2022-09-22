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
    \brief Arduino SPI pin settings
*/

#define MURATA_ASIC_UNO_CS     8    
#define MURATA_ASIC_DUE_CS    11
#define W5X00_SD_CS_PIN        4
#define W5X00_ETHERNET_CS_PIN 10

///@}


///@{
/*!
    \brief IMU settings
*/

#define IMU_SAMPLING_RATE 500 // sampling rate, n.b limitations with transfer speed
#define BUFFER_SIZE 2 // IMU buffer size, n.b limitations with transfer speed
#define STRUCT_SIZE 26  // 64 + 8 * 16 + 4 * 4

// // filter parameters, possible values 13,20,46,200,300
// #define GYRO_FILTER 46
// #define ACC_FILTER  46

///@}


///@{
/*!
    \brief Camera trigger settings
*/

#define CAM_TRIGGER_RATE 10 // camera trigger rate
#define CAM_TRIGGER_LENGTH 100 // camera trigger length in microseconds
#define CAM_TRIGGER_PIN 13 // camera trigger pin 

///@}


///@{
/*!
    \brief GNSS timestamp and pin settings 
*/

#define GNSS_IN_USE 0
#define GNSS_INPUT_PIN 25 // Set interrupt pin

///@}


///@{
/*!
    \brief UDP data transfer and connection settings

    check networking.md for details on connecting client and server
*/

#define UDP_DEVICE_MAC  { 0x98, 0xFA, 0x9B, 0xF0, 0xD6, 0xF6 }
#define UDP_SUBNET      { 255, 255, 255, 0 }
#define UDP_DEVICE_IP   { 192, 168, 2, 1 }
#define UDP_GATEWAY_IP  { 192, 168, 2, 2 }
#define UDP_SERVER_IP   { 192, 168, 2, 2 }

#define UDP_SERVER_PORT 5005
#define PACKET_SIZE_STARTUP 1000

///@}


#endif // CONFIG_H