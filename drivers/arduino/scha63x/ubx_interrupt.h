#ifndef UBX_INTERRUPT_H
#define UBX_INTERRUPT_H

#include <Arduino.h>

/*!
    @file ubx_interrupt.h
    @brief uBlox timepulse interrupt setting and handling 
*/

void setUBXInterrupt(void function(void));

#endif