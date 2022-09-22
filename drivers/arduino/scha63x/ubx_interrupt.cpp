#include <stdio.h>

#include "config.h"
#include "ubx_interrupt.h"

/*!
    @file ubx_interrupt.cpp
    @brief uBlox timepulse interrupt setting and handling 
*/

/*!
    \brief interrupt callback function holder
*/
static void (*ubx_callback)(void);

/*!
    \brief Sets Arduino pin for uBlox triggers
*/
void setupUBXPin(void) {
    pinMode( GNSS_INPUT_PIN, INPUT );
}

/*!
    \brief Set pin interrupt and callback
*/ 
void setUBXInterrupt(void function(void)) {
    setupUBXPin();
    attachInterrupt(digitalPinToInterrupt(GNSS_INPUT_PIN), function, RISING);
}