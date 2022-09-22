#ifndef ARDUINO_UDP_H
#define ARDUINO_UDP_H

#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#include <stdint.h>
#include <stdbool.h>

/*!
    @file arduino_udp.h
    @brief UDP communication with server
*/

/*!
    \brief sensor data, same declaration on server side definitions.h
*/
typedef struct _sensor_data {
    
    int sensitivity_acc;
    int sensitivity_gyro_x;
    int sensitivity_gyro_y;
    int sensitivity_gyro_z;

    int data_buffer_size;
    
} sensor_data;

IPAddress udp_init(void);
int startUpSeq(IPAddress& address);
int sendImuInfo(IPAddress& address);
void sendUDPpacketWithContent(IPAddress& address, unsigned char* packetBuffer, unsigned int packet_size);
byte* getUDPpacketWithContent(unsigned char* packetBuffer, unsigned int packet_size);


#endif // #ifndef ARDUINO_UDP
