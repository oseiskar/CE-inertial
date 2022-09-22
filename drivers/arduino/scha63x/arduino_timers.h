#ifndef TIMERS_H
#define TIMERS_H

#include <Arduino.h>
#include <config.h>

#include "stdint.h"

/*!
    @file arduino_timers.h
    @brief Timer API for Arduino

    Timers for different Arduino models defined in 
    ../murata/properties folder
*/

void MsTimer_Initialize(void);
void SampleTimer_Initialize(void function(void));
void CamTrigger_Initialize(void function(void));
uint64_t TimeStamp_Initialize(void);

void SampleTimer_Restart(void);
void SampleTimer_Stop(void);

void Wait_ms(volatile uint32_t ms);
uint64_t Get_sec(void);
uint64_t Get_ms(void);
uint64_t Get_microsec(void);

#endif
