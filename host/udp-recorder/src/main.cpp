/*!
    @file main.cpp
    @brief UDP server

    Protocol buffers (e.g. https://github.com/protocolbuffers/protobuf)
    could be used in data serialization for better compability, predefined 
    structs sent directly without protocol buffers are used for now
*/

#include <iostream>
#include <sstream>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <jsonl-recorder/recorder.hpp>

#include "defs.h"
#include "config.h"
#include "conversion.h"


/*! \brief microseconds in second*/
#define micros 1000000

///@{
/*! \brief Macro */
#define clear(buffer) memset(buffer, 0, sizeof(buffer));
#define clear_s(buffer) memset(&buffer, 0, sizeof(buffer));
#define resp_print(buffer, from)                                \
    {                                                           \
        printf("%s from IP:%s, Port:%hu\r\n", &buffer[0],         \
               inet_ntoa(from.sin_addr), ntohs(from.sin_port)); \
    }
///@}


/*!
    \brief Cast current system timestamp into string

    Used in .jsonl file header generation

    \return string of system timestamp
*/
std::string currentISO8601TimeUTC()
{
    auto now = std::chrono::system_clock::now();
    auto itt = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(gmtime(&itt), "%FT%H-%M-%SZ");
    return ss.str();
}

/*!
    \brief Receive UDP packet from socket

    Blocking function, 

    \param sock     bound socket
    \param recBuf   pointer to a char buffer for storing the response from client
    \param buffer_s size of the buffer to be sent
    \param from     address of the client
    \param fromlen  length 
*/
template <typename T>
int receivePacket(_udp connection, int buffer_s, T *recBuf)
{
    int n = recvfrom(connection.sock,
                     recBuf, buffer_s, 0,
                     (struct sockaddr *)&connection.from,
                     &connection.fromlen);
    if (n < 0)
        std::runtime_error("Can not receive in server!");

    return 0;
}

/*!
  \brief Send UDP packet to client

  Blocking while buffer is not full 

  \param sock     bound socket
  \param recBuf   pointer to a char buffer for storing the response from client
  \param buffer_s size of the buffer to be sent
  \param from     address of the client
  \param fromlen  length 

*/
template <typename T>
int sendPacket(_udp connection, int buffer_s, T *recBuf)
{
    int n = sendto(connection.sock,
                   recBuf, buffer_s, 0,
                   (const struct sockaddr *)&connection.from,
                   connection.fromlen);
    if (n < 0)
        std::runtime_error("Can not send from client");

    return 0;
}

/*!
    \brief Setup UDP variables and connection parameters

    Hardcoded protocol, type and protocol value

    \return struct containing all variables needed when communicating with client
    \exception socket creation failed, throws std::runtime_error EXIT_FAILURE
    \exception binding socket failed, throws std::runtime_error EXIT_FAILURE
*/
_udp setupUDP(void)
{
    int sock;
    socklen_t fromlen;
    struct sockaddr_in server; // Server sockaddr structure
    struct sockaddr_in from;   // Client sockaddr structure

    // Unbound socket creation
    sock = socket(AF_INET, SOCK_DGRAM, 0); // Protocol (IPv4), Type (UDP), Protocol value for IP (0)
    if (sock < 0)
    {
        throw std::runtime_error("Can not create socket in server");
    }
    memset(&server, 0, sizeof(struct sockaddr_in)); // clear server struct

    server.sin_family = AF_INET;         // IPv4
    server.sin_port = htons(port);       // port with byte order converted (big-endian)
    server.sin_addr.s_addr = INADDR_ANY; // server address 0.0.0.0

    // Bind Socket to a local address, returns 0 if no errors
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        throw std::runtime_error("Can not bind in server!");
    }
    memset(&from, 0, sizeof(struct sockaddr_in)); // clear from struct
    fromlen = sizeof(struct sockaddr_in);         // internet style socket address length

    _udp connection;
    connection.sock = sock;
    connection.from = from;
    connection.fromlen = fromlen;

    return connection;
}

/*!
  \brief generate IMU's filter config from config.h

  Currently hardcoded values, should be read from a config file

  \return struct of filter values, structure defined in defs
*/
scha63x_sensor_config generateConfig(void)
{
    scha63x_sensor_config sensor_config;

    sensor_config.acc_filter.Ax = Acc_Ax;
    sensor_config.acc_filter.Ay = Acc_Ay;
    sensor_config.acc_filter.Az = Acc_Az;
    sensor_config.gyro_filter.Rz2_Rx2 = Gyro_Rz2_Rx2;
    sensor_config.gyro_filter.Rz_Rx = Gyro_Rz_Rx;
    sensor_config.gyro_filter.Ry2 = Gyro_Ry2;
    sensor_config.gyro_filter.Ry = Gyro_Ry;

    return sensor_config;
}



int main(void)
{
    try
    {
        /* JSONL Recorder initialization */

        auto startTimeString = currentISO8601TimeUTC();
        auto outputPrefix = "output/recording-" + startTimeString;
        auto recorder = recorder::Recorder::build(outputPrefix + ".jsonl");


        /* Setup connections */

        _udp connection = setupUDP();


        /* Connection startup */

        // PING : "hello"
        char recBuf[buffer_size]; // receive buffer
        char outBuf[buffer_size]; // compare buffer
        sprintf(outBuf, "hello");

        printf("Waiting for packet!\n");
        while (1) // Might not be needed since receive is blocking
        {
            receivePacket(connection, sizeof(recBuf), &recBuf);
            resp_print(recBuf, connection.from);
            // if received packet buffer has "hello" then ping and continue
            if (strcmp(recBuf, outBuf) == 0)
            {
                sendPacket(connection, buffer_size, &recBuf);
                break;
            }
        }

        // CONFIG : send IMU config
        scha63x_sensor_config config = generateConfig();
        sendPacket(connection, buffer_size, &config);

        // STATUS : Receive status update
        sensor_data specs;
        receivePacket(connection, buffer_size, &specs);

        // SAMPLE BUFFER : send sample buffer and trigger info
        int data_buffer = imu_buffer_size;
        specs.buffer = imu_buffer_size;
        specs.imu_trigger = imu_trigger_rate;
        specs.cam_trigger = cam_trigger_rate;
        sendPacket(connection, buffer_size, &specs);

        // CAC TERMS : receive cross-axis terms
        scha63x_cacv cac_values;
        receivePacket(connection, buffer_size, &cac_values); // size was 72
        cacvValues(cac_values);

        // TIMESTAMP might not be needed
        unsigned long firstTimeStamp;
        clear(recBuf);
        receivePacket(connection, buffer_size, &recBuf);
        sscanf(recBuf, "%lu", &firstTimeStamp);
        sendPacket(connection, buffer_size, &recBuf);

        /* INIT COMPLETE */


        /* Start receiving sampled raw data packets */

        const int receive_size = struct_size * data_buffer;
        scha63x_raw_data data_vector[data_buffer];
        scha63x_real_data scha63x_data;

        while (1) 
        {
            fflush(stdout);
            receivePacket(connection, receive_size, data_vector);

            if (int(data_vector->timeStamp) != 0)
            {
                for (int i = 0; i < data_buffer; i++)
                {
                    // Timestamping should be checked, first timestamp would be positive 0
                    unsigned long timeStamp1 = data_vector[i].timeStamp - firstTimeStamp;
                    float timeStamp = 1.0 * timeStamp1 / micros;

                    // data conversion
                    scha63x_convert_data(&data_vector[i], &scha63x_data); // convert LSB values to float
                    scha63x_cross_axis_compensation(&scha63x_data);       // cross-axis compensation

                    // IMU data, gyro & accel
                    recorder->addGyroscope(timeStamp, \
                        scha63x_data.gyro_x, scha63x_data.gyro_y, scha63x_data.gyro_z);
                    recorder->addAccelerometer(timeStamp, \
                        scha63x_data.acc_x, scha63x_data.acc_y, scha63x_data.acc_z);

                    // CAM frame 
                    if (data_vector[i].cam_trigger)
                    {
                        std::vector<recorder::FrameData> frameGroup;
                        for (int index = 0; index < 2; index++)
                        { 
                            recorder::FrameData frameData({ .t = timeStamp, .cameraInd = index });
                            frameGroup.push_back(frameData);
                        }
                        recorder->addFrameGroup(frameGroup[0].t, frameGroup);
                    }
                }

                clear_s(scha63x_data);
                clear(data_vector);
            }
        }
    }

    catch (std::runtime_error &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    catch ( ... )
    {
        std::cerr << "Unknown error" << '\n';
        return EXIT_FAILURE;       
    }

    return EXIT_SUCCESS;
}