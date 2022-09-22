#include <SPI.h>

#include <Ethernet.h>
#include <EthernetUdp.h>

#include <config.h>
#include <scha63x_driver.h>
#include <scha63x_spi.h>
#include <ubx_interrupt.h>
#include <arduino_udp.h>
#include <arduino_timers.h>

/*!
    @file murata.ino
    @brief Main program loop
*/


#define ATOMIC_BLOCK_FORCEON AtomicBlockForceOn atomicBlockForceOn_

class AtomicBlockForceOn
{
public:
    // Constructor: called when the object is created
    inline AtomicBlockForceOn()
    {
        noInterrupts(); // turn interrupts OFF
    }

    // Destructor: called when the object is destroyed (ex: goes
    // out-of-scope)
    inline ~AtomicBlockForceOn()
    {
        interrupts(); // turn interrupts ON
    }
};

/*!
    \brief data vector for storing raw data from scha63x
*/
scha63x_raw_data data_vector[BUFFER_SIZE];

/*!
    \brief index for data buffer, looping in main sampling loop
*/
static volatile int buffer_index = 0;

/*!
    \brief number of bytes in data buffer, used in sending UDP packets
*/
static int buf_bytes = BUFFER_SIZE * STRUCT_SIZE;

/*!
    \brief server IP address
*/
IPAddress udp_server;


///@{
/*! \brief Error flag */
static volatile bool DueError = false;
static volatile bool UnoError = false;
///@}

///@{
/*! \brief Interrupt flag */
volatile bool imu_sampling_flag = false;
volatile bool cam_trigger_flag = false;
volatile bool ubx_trigger_flag = false;
volatile bool reset_flag = false;
///@}

///@{
/*! \brief Timestamp */
volatile uint64_t imu_timestamp;
volatile uint64_t cam_timestamp;
volatile uint64_t ubx_timestamp;
///@}

/*! \brief Number of microseconds */
volatile long int microsec_counter;


/*!
    \brief Callback function for data sampling via SPI 
*/
void imu_sampling_callback(void)
{
//  Serial.println("IMU");
  imu_sampling_flag = true;
}

/*!
    \brief Callback function for camera triggers
*/
void cam_trigger_callback(void)
{
//  Serial.println("CAM");
  cam_trigger_flag = true;
}

/*!
    \brief Callback for UBX interrupt
*/
void ubx_callback(void) 
{
//  Serial.println("GNSS");
  ubx_trigger_flag = true;
//  ubx_timestamp = micros();
}

/*!
    \brief Reset function
*/
void reset_callback(void)
{
  //resetFunc();
  reset_flag = true;
}


void setup()
{
  digitalWrite( 45 , HIGH );
  delay(200); 
  pinMode(45, OUTPUT);

  pinMode( 41, INPUT_PULLUP );  
  delay(200); 
  attachInterrupt(digitalPinToInterrupt( 41 ), reset_callback, LOW);
  reset_flag = 0;
  
  Serial.begin(115200);
  Serial.println("Started");

  MsTimer_Initialize();
  SPI_Initialize();

  while (true) {
    Wait_ms(500);
    if (reset_flag == true) {
      Serial.println("trigger");
      delay(200); 
      digitalWrite( 45 , LOW );
    }
  }
  
  // Initialize UDP, receive IMU config
  udp_server = udp_init();
  startUpSeq(udp_server);


  // Filter config, structs from UDP

  struct filters filter;
  scha63x_sensor_config sensor_config;

  sensor_config.acc_filter.Ax = filter.FILTER_46HZ;
  sensor_config.acc_filter.Ay = filter.FILTER_46HZ;
  sensor_config.acc_filter.Az = filter.FILTER_46HZ;

  sensor_config.gyro_filter.Rz2_Rx2 = filter.FILTER_46HZ;
  sensor_config.gyro_filter.Rz_Rx = filter.FILTER_46HZ;
  sensor_config.gyro_filter.Ry2 = filter.FILTER_46HZ;
  sensor_config.gyro_filter.Ry = filter.FILTER_46HZ;

  char serial_num[14];
  int status;
  
  status = initialize_sensor(serial_num, &sensor_config);
  
  if (status != SCHA63X_OK) {
    Serial.print("ERROR: ");
    Serial.println(status);
    while (true);
  }

  Serial.println(serial_num);
  Serial.println("!!! Initialization Complete !!!");

  // Clear data_vector
  memset(&data_vector, 0, sizeof(data_vector));

  pinMode(CAM_TRIGGER_PIN, OUTPUT);
  digitalWrite(CAM_TRIGGER_PIN, LOW);

  // Send sensor information from config and cross-axis terms over UDP
  // status and serial number needs to be added to struct
  sendImuInfo(udp_server);

  // Start sampling
  SampleTimer_Initialize(imu_sampling_callback);
  CamTrigger_Initialize(cam_trigger_callback);
  setUBXInterrupt(ubx_callback);

} // setup


void loop()
{

  // trigger should be between 1us and 1ms, set in config
  if( (digitalRead(CAM_TRIGGER_PIN) == HIGH) && \
                (Get_microsec() - microsec_counter >= CAM_TRIGGER_LENGTH) ) {
    digitalWrite(CAM_TRIGGER_PIN, LOW);
  }

  if (imu_sampling_flag) {
    
    imu_timestamp = micros();
    imu_sampling_flag = false;

    if (cam_trigger_flag) {
//      Serial.println("T");
      microsec_counter = micros();
      digitalWrite(CAM_TRIGGER_PIN, HIGH);
    }

    static scha63x_raw_data scha63x_raw_data_last;
    static int32_t num_samples = 0;

    { // scope for reading sensor
      ATOMIC_BLOCK_FORCEON;
      scha63x_read_data(&scha63x_raw_data_last);
    
      data_vector[buffer_index].timeStamp = imu_timestamp;
      data_vector[buffer_index].acc_x_lsb = scha63x_raw_data_last.acc_x_lsb;
      data_vector[buffer_index].acc_y_lsb = scha63x_raw_data_last.acc_y_lsb;
      data_vector[buffer_index].acc_z_lsb = scha63x_raw_data_last.acc_z_lsb;
      data_vector[buffer_index].gyro_x_lsb = scha63x_raw_data_last.gyro_x_lsb;
      data_vector[buffer_index].gyro_y_lsb = scha63x_raw_data_last.gyro_y_lsb;
      data_vector[buffer_index].gyro_z_lsb = scha63x_raw_data_last.gyro_z_lsb;
      data_vector[buffer_index].temp_due_lsb = scha63x_raw_data_last.temp_due_lsb;
      data_vector[buffer_index].temp_uno_lsb = scha63x_raw_data_last.temp_uno_lsb;
  
      // Triggers 
      if (cam_trigger_flag) {
        data_vector[buffer_index].cam_trigger = true;
        cam_trigger_flag = false;
      } else {
        data_vector[buffer_index].cam_trigger = false;
      }
  
      if (ubx_trigger_flag) {
        data_vector[buffer_index].ubx_trigger = true;
        ubx_trigger_flag = false;
      } else {
        data_vector[buffer_index].ubx_trigger = false;
      }
    }
    
    if (scha63x_raw_data_last.rs_error_due) {
      DueError = true;
    }
    if (scha63x_raw_data_last.rs_error_uno) {
      UnoError = true;
    }

    if (++buffer_index >= BUFFER_SIZE) {
      
      ATOMIC_BLOCK_FORCEON;
      
      // digitalWrite may not be necessary
      digitalWrite(W5X00_ETHERNET_CS_PIN, LOW);
      sendUDPpacketWithContent(udp_server, (byte *) data_vector, buf_bytes);
      digitalWrite(W5X00_ETHERNET_CS_PIN, HIGH);
      
      // Housekeeping
      memset(&data_vector, 0, sizeof(data_vector));
      buffer_index = 0;
    }    
  }

  if (DueError) {
    SampleTimer_Stop();
    DueError = false;

    scha63x_sensor_status status;
    scha63x_read_sensor_status_due(&status); // This clears sensor RS (Return Status) errors

    if (!UnoError) {
      SampleTimer_Restart();
    }

  }

  if (UnoError) {
    SampleTimer_Stop();
    UnoError = false;

    scha63x_sensor_status status;
    scha63x_read_sensor_status_uno(&status); // This clears sensor RS errors

    if (!DueError) {
      SampleTimer_Restart();
    }
    
  }

} // loop
