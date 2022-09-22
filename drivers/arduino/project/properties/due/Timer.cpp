#include "arduino_timers.h"
#include <DueTimer.h>

#include "stdbool.h"

/*!
    @file Timer.cpp
    @brief timer declarations for Arduino DUE

    using DueTimer library
*/

/*!
    \brief Microsecond counter

    32-bit overflows in around 70 minutes (2^32 / 10^6 / 60), 
    64-bit unsigned int is used
*/
static volatile uint64_t timerCnt = 0; 

///@{
/*!
    \brief Callback holders
*/
static void (*imu_callback)(void);
static void (*cam_callback)(void);
///@}


// Getters

///@{
/*!
    \brief Time getters
*/
uint64_t Get_microsec(void) { return micros(); }
uint64_t Get_ms(void) { return timerCnt; }
uint64_t Get_sec(void) { return Get_ms() / 1000; }
///@}

/*!
    \brief Initial timestamp getter
*/
uint64_t TimeStamp_Initialize(void)
{
    return Get_microsec();
}

/*!
    \brief Convenience function for millisecond timer

    Optimization level set to -O0 (no optimization) to prevent 
    optimization of delay function. Existing optimization level
    is saved and restored. Used in sensor initialization. 
*/
#pragma push // Save existing optimization level
#pragma O0   // Optimization level now -O0 (no optimization)
void Wait_ms(volatile uint32_t ms)
{                   
    uint32_t tickstart = Get_ms();
    while((Get_ms() - tickstart) < ms)
    {
    }  
}
#pragma pop // Restore original optimization level



// Timer callbacks and manipulation

/*!
    \brief Timer interrupt handler for TIM4 

    Optimization level set to -O0 (no optimization) to prevent 
    optimization of delay function. Existing optimization level
    is saved and restored. 
*/
#pragma push // Save existing optimization level
#pragma O0   // Optimization level now -O0 (no optimization)
void TIM4_handler(void)
{
    timerCnt++;
}
#pragma pop // Restore original optimization level

/*!
    \brief Timer 4 interrupt handler

    calls the callback function given at the initialization 
    of timers
*/
void TIM3_handler(void) { imu_callback(); }

/*!
    \brief Timer 5 interrupt handler

    calls the callback function given at the initialization 
    of timers
*/
void TIM5_handler(void) { cam_callback(); }


/*!
    \brief Timer 3 (sample timer) restart function
*/
void SampleTimer_Restart(void) { Timer3.start(); }

/*!
    \brief Timer 3 (sample timer) stop function
*/
void SampleTimer_Stop(void) { Timer3.stop(); }



// Timer initializations

/*!
    \brief Timer 4 (camera timer) initializer

    Increases the timerCnt global variable, callback given at initialization
*/
void MsTimer_Initialize(void)
{
    Timer4.attachInterrupt(TIM4_handler).start(1000);
}

/*!
    \brief Timer 3 (sample timer) initializer

    IMU trigger rate defined in config.h

    \param function IMU callback function
*/
void SampleTimer_Initialize(void function(void))
{
    Timer3.attachInterrupt(TIM3_handler).setFrequency(IMU_SAMPLING_RATE).start();
    imu_callback = function;
}

/*!
    \brief Timer 5 (camera timer) initializer

    Camera trigger rate defined in config.h

    \param function Cam callback function
*/
void CamTrigger_Initialize(void function(void))
{
    Timer5.attachInterrupt(TIM5_handler).setFrequency(CAM_TRIGGER_RATE).start();
    cam_callback = function;
}
