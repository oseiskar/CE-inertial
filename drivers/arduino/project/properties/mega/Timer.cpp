#include "arduino_timers.h"

#include "stdbool.h"

/*!
    @file Timer.cpp
    @brief timer declarations for Arduino MEGA

    Currently lacking functionality, use Arduino DUE board if possible (custom Murata
    boards will also work with Due out of the box)
    
    TODOs
    * dynamically change sample timer rate from config.h
    * camera trigger timer and interrupt
    * noInterrupts is probably not the best way of stopping timers
*/

/*!
    \brief Microsecond counter

    32-bit overflows in around 70 minutes (2^32 / 10^6 / 60)
*/
static volatile uint32_t timerCnt = 0; 


// Getters

///@{
/*!
    \brief Time getters
*/
uint32_t Get_sec(void) { return Get_ms() / 1000; }
uint32_t Get_ms(void) { return timerCnt; }
///@}


/*!
    \brief Callback holder for IMU
*/
static void (*callback)(void);


/*! 
    \brief Sample timer compare variable

    AtMega2560 timer documentation ch. 17

    SAMPLE timer 16-bit CLOCK 3 register TCCR3C 
    rate = system freq / prescalar / (compare value + 1)
                = 16 MHz / 1 / (6152 + 1) = 2600.357 Hz
*/
const uint16_t sample_comp = 625; //15625; // testing 6152; // 100 Hz

/*! 
    \brief Millisecond timer compare variable

    AtMega2560 timer documentation ch. 17

    MILLIS timer 16-bit CLOCK 4 register TCCR4C
    rate = system freq / prescalar / (compare value + 1)
                = 16 MHz / 1 / (15999 + 1) = 1000.00 Hz
*/
const uint16_t millis_comp = 15999;




// Timer initialization

/*!
    \brief Timer 3 (sample timer) initializer
*/
static void TIM3_Timer_Init(void)
{
    TCCR3A = 0; // clear Control Register A
    TCCR3B = 0; // clear Control Register B, no need to reset bits
    TCNT3  = 0; // clear timer 3 (upper (H) and lower (L) registers)

    OCR3A = sample_comp;     // compare value
    TCCR3B |= (1 << WGM32);  // operation mode CTC with OCRnA match
    // TCCR3B |= (1 << CS32) | (1 << CS30);   // prescaler of 1024
    TCCR3B |= (1 << CS32); // prescaler of 256
    // TCCR3B |= (1 << CS30);   // prescaler of 1
    TIMSK3 |= (1 << OCIE3A); // enable compare interrupt

    sei(); // enable global interrupts
}


/*!
    \brief Stop timer 3 (sample timer)
*/
static void TIM3_Timer_Stop(void)
{
    // // reset control registers (prescalar and op. mode)
    // TCCR3A = 0; // clear Control Register A
    // TCCR3B = 0; // clear Control Register B
    // TCNT3  = 0; // clear timer 3 (upper (H) and lower (L) registers)
    // TIMSK3 = 0; // disable interrupt
    noInterrupts();

}

/*!
    \brief Timer 4 initializer
*/
static void TIM4_Timer_Init(void)
{  
    TCCR4A = 0; // clear Control Register A
    TCCR4B = 0; // clear Control Register B, no need to reset bits
    TCNT4  = 0; // clear timer 4 (upper (H) and lower (L) registers)

    OCR4A = millis_comp;     // compare value
    TCCR4B |= (1 << WGM42);  // operation mode CTC with OCRnA match
    TCCR4B |= (1 << CS40);   // prescaler of 1
    TIMSK4 |= (1 << OCIE4A); // enable compare interrupt

    sei(); // enable global interrupts
    
    
    // SHOULD BE REMOVED 

    // TCCR1A = 0; // clear Control Register A
    // TCCR1B = 0; // clear Control Register B, no need to reset bits
    // TCNT1  = 0; // clear timer 4 (upper (H) and lower (L) registers)

    // OCR1A = millis_comp;     // compare value
    // TCCR1B |= (1 << WGM12);  // operation mode CTC with OCRnA match
    // TCCR1B |= (1 << CS10);   // prescaler of 1
    // TIMSK1 |= (1 << OCIE1A); // enable compare interrupt

    // sei(); // enable global interrupts
}

/*!
    \brief TIM3 Handler (sampling timer)
*/
ISR(TIMER3_COMPA_vect) 
{
    callback();
}

/*!
    \brief TIM4 Handler (millisecond timer)

    Optimization level set to -O0 (no optimization) to prevent 
    optimization of delay function. Existing optimization level
    is saved and restored. 
*/

// Prevent optimization of timer handler
#pragma push // Save existing optimization level
#pragma O0   // Optimization level now -O0 (no optimization)
ISR(TIMER4_COMPA_vect) 
{
    timerCnt++;
}
#pragma pop // Restore original optimization level



// Timer initializations


/*!
    \brief Timer Initialization for millisecond timer
*/
void MsTimer_Initialize(void)
{
    TIM4_Timer_Init();
}

/*!
    \brief Timer Initialization for sampling timer
*/
void SampleTimer_Initialize(void function(void))
{
    TIM3_Timer_Init();
    callback = function;
}

/*!
    \brief Restart sampling timer
*/
void SampleTimer_Restart(void)
{
    // TIM3_Timer_Init();
    interrupts();
}

/*!
    \brief Stop sampling timer
*/
void SampleTimer_Stop(void)
{
    TIM3_Timer_Stop();
}


/*!
    \brief Convenience function for millisecond timer

    Optimization level set to -O0 (no optimization) to prevent 
    optimization of delay function. Existing optimization level
    is saved and restored. Used in sensor initialization. 
*/
// Prevent optimization of delay function
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

