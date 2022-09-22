#include <stdio.h>

#include "arduino_udp.h"
#include "arduino_timers.h"
#include "scha63x_driver.h"
#include "config.h"
#include "defs.h"

/*!
    @file arduino_udp.cpp
    @brief UDP communication with server
*/


/*!
    \brief Arduino Udp object
*/
EthernetUDP Udp;


///@{
/*!
    \brief data buffer
*/
byte packetBuffer[PACKET_SIZE_STARTUP]; // received data
byte outBuffer[PACKET_SIZE_STARTUP];    // sent data
byte imuConfig[1000];                    // buffer for IMU filter values
///@}


///@{
/*!
    \brief UDP addresses and port
*/
IPAddress ip(UDP_DEVICE_IP);
IPAddress gateway(UDP_GATEWAY_IP);
IPAddress subnet(UDP_SUBNET);
IPAddress UDPServer(UDP_SERVER_IP);
byte mac[] = UDP_DEVICE_MAC;
unsigned int UDPport = UDP_SERVER_PORT;
///@}


/*!
    \brief Initialize connection with server device

    Handles pin setting and starting Ethernet object

    \return IP address of the server
*/
IPAddress udp_init(void)
{
    digitalWrite(W5X00_ETHERNET_CS_PIN, LOW);
    Ethernet.begin(mac, ip, gateway, gateway, subnet);
    Udp.begin(UDPport);
    digitalWrite(W5X00_ETHERNET_CS_PIN, HIGH);
    
    return UDPServer;
}


/*!
    \brief Arduino startup sequence with server

    Connects to server, pings and receives IMU filter settings 
    used in IMU initialization

    \param address server address
    \return integer, success 1, failure 0 
*/
int startUpSeq(IPAddress &address)
{
    memset(packetBuffer, 0, PACKET_SIZE_STARTUP);
    memset(outBuffer, 0, PACKET_SIZE_STARTUP);
    sprintf((char *)packetBuffer, "hello");
    byte *i;
    
    memset(imuConfig, 0, 1000);
    //scha63x_sensor_config config;
    
    // ping 
    while (true)
    {
        sendUDPpacketWithContent(address, packetBuffer, PACKET_SIZE_STARTUP);
        delay(200);
        byte *i = getUDPpacketWithContent(outBuffer, PACKET_SIZE_STARTUP);
        Serial.println((char *)i);
        if (strcmp((char *)i, (char *)packetBuffer) == 0)
        {
            Serial.println("PING OK");
            return 1;
            
            // Receive IMU config, DOES NOT WORK
            byte *i = getUDPpacketWithContent((byte *) imuConfig, 1000);
            //Udp.readPacket((byte *) &config, 1000, ip, port);
            Serial.println((char *)i);
            //return *i;
            
        }
    }
}

/*!
    \brief Share information about IMU after IMU initialization

    Sends sensor type, buffer size cross-axis terms and timestamp

    \param address server address
    \return integer, success 1, failure 0 
*/
int sendImuInfo(IPAddress& address)
{
   //////

    // // send sample buffer info
    // memset(packetBuffer, 0, PACKET_SIZE_STARTUP);
    // sprintf((char *)packetBuffer, "%u", BUFFER_SIZE);

    // sensor_data sensor;
    // sensor.sensitivity_acc = SENSITIVITY_ACC; 
    // sensor.sensitivity_gyro_x = SENSITIVITY_GYRO_X;
    // sensor.sensitivity_gyro_y = SENSITIVITY_GYRO_Y;
    // sensor.sensitivity_gyro_z = SENSITIVITY_GYRO_Z;
    // sensor.data_buffer_size = BUFFER_SIZE;

    // sendUDPpacketWithContent(address, packetBuffer, PACKET_SIZE_STARTUP);
    // delay(10);
    // byte *a = getUDPpacketWithContent(outBuffer, PACKET_SIZE_STARTUP);
    
    // // Serial.println((char *)a);
    // // Serial.println((char *)packetBuffer);
    
    // if (strcmp((char *)a, (char *)packetBuffer) == 0)  {
    //     Serial.println("SAMPLE INFO OK");
    // } 

    //////
    

    // // send sensor type
    // memset(packetBuffer, 0, PACKET_SIZE_STARTUP);
    // sprintf((char *)packetBuffer, "%u", BUFFER_SIZE);
    // sendUDPpacketWithContent(address, packetBuffer, PACKET_SIZE_STARTUP);
    // delay(10);
    // byte *a = getUDPpacketWithContent(outBuffer, PACKET_SIZE_STARTUP);

    // // Serial.println((char *)g);
    // // Serial.println((char *)packetBuffer);

    // if (strcmp((char *)a, (char *)packetBuffer) == 0)  {
    //     Serial.println("SENSOR TYPE OK");
    // } 

    // delay(10);

    // // send sensor type
    // memset(packetBuffer, 0, PACKET_SIZE_STARTUP);
    // sprintf((char *)packetBuffer, "%u", SENSOR_TYPE);
    // sendUDPpacketWithContent(address, packetBuffer, PACKET_SIZE_STARTUP);
    // delay(10);
    // byte *g = getUDPpacketWithContent(outBuffer, PACKET_SIZE_STARTUP);

    // // Serial.println((char *)g);
    // // Serial.println((char *)packetBuffer);

    // if (strcmp((char *)g, (char *)packetBuffer) == 0)  {
    //     Serial.println("SENSOR TYPE OK");
    // } 

    // delay(10);

    // send cross-axis terms
    scha63x_cacv *cacv = get_cacv_ptr();
    sendUDPpacketWithContent(address, (byte *) cacv, 1000); //72);
    delay(10);
    // byte *z = getUDPpacketWithContent(outBuffer, PACKET_SIZE_STARTUP);
    Serial.println("CROSS AXIS DONE");


    // // send timestamp 
    // memset(packetBuffer, 0, PACKET_SIZE_STARTUP);
    // sprintf((char *)packetBuffer, "%lu", Get_microsec());
    // sendUDPpacketWithContent(address, packetBuffer, PACKET_SIZE_STARTUP);
    // delay(10);
    // byte *u = getUDPpacketWithContent(outBuffer, PACKET_SIZE_STARTUP);
    //     if (strcmp((char *)u, (char *)packetBuffer) == 0)  {
    //     Serial.println("TIMESTAMP OK");
    // } 
    
    return 0;
}


/*!
    \brief Send sensor data packet to server

    \param address server address
    \param packetBuffer pointer to data to be sent
    \param packet_size size of packet to be sent, depends on the sample buffer size   
*/
void sendUDPpacketWithContent(IPAddress &address, unsigned char *packetBuffer, unsigned int packet_size)
{
    Udp.beginPacket(address, UDPport);
    Udp.write(packetBuffer, packet_size); // number of chars sent could be returned
    Udp.endPacket();
}


/*!
    \brief Receive packet to server

    Used only before data logging

    \param packetBuffer pointer to data to be sent
    \param packet_size size of packet to be sent, depends on the sample buffer size   
*/
byte* getUDPpacketWithContent(unsigned char *packetBuffer, unsigned int packet_size)
{
    if (Udp.parsePacket())
    {

        if (Udp.remoteIP() != UDPServer)
        {
            Serial.println(F("UDP IP Bad"));
            return 0;
        }

        if (Udp.remotePort() != UDPport)
        {
            Serial.println(F("Port Bad"));
            return 0;
        }

        Udp.read(packetBuffer, packet_size);
        return packetBuffer;
    }
}
