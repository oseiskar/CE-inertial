#include "arduino_timers.h"

/*!
    @file arduino_timers.cpp
    @brief Timer API for Arduino

    Timers for different Arduino models defined in 
    Arduino .ino folder
*/

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    /*! \brief timers for Mega2560 */
    #include "../murata/properties/mega/Timer.cpp" 
#else
    /*! \brief timers for Due */
    #include "../murata/properties/due/Timer.cpp" 
#endif


