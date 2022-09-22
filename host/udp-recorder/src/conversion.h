#ifndef CONVERSION_H
#define CONVERSION_H

#include <stdint.h>
#include <stdbool.h>

#include "defs.h"

/*!
    @file conversion.h
    @brief cross-axis conversion
*/


void cacvValues(scha63x_cacv values);
void scha63x_convert_data(scha63x_raw_data *data_in, scha63x_real_data *data_out);
void scha63x_cross_axis_compensation(scha63x_real_data *data);

#endif