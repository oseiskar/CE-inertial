/*!
    @file conversion.cpp
    @brief cross-axis conversion
*/

#include "conversion.h"
#include "config.h"

/*! \brief temperature calculation macro */
#define GET_TEMPERATURE(temp) (25 + ((temp) / 30.0))

/*!
    \brief Internal data structure, Cross-axis compensation values
*/
static scha63x_cacv scha63x_cac_values; 

/*!
    \brief Caller function for setting CAC values
*/
void cacvValues(scha63x_cacv values){
    scha63x_cac_values = values;
}

/*!
    \brief Convert summed raw binary data from sensor to real values. 
    Also calculate averages values.

    \param data_in  pointer to summed "raw" data from sensor
    \param data_out pointer to converted values
*/
void scha63x_convert_data(scha63x_raw_data *data_in, scha63x_real_data *data_out)
{
    data_out->acc_x = data_in->acc_x_lsb;
    data_out->acc_y = data_in->acc_y_lsb;
    data_out->acc_z = data_in->acc_z_lsb;
    data_out->gyro_x = data_in->gyro_x_lsb;
    data_out->gyro_y = data_in->gyro_y_lsb;
    data_out->gyro_z = data_in->gyro_z_lsb;
    
    // Convert from LSB to sensitivity and calculate averages here for faster execution
    data_out->acc_x = data_out->acc_x / (SENSITIVITY_ACC);
    data_out->acc_y = data_out->acc_y / (SENSITIVITY_ACC);
    data_out->acc_z = data_out->acc_z / (SENSITIVITY_ACC);
    data_out->gyro_x = data_out->gyro_x / (SENSITIVITY_GYRO_X);
    data_out->gyro_y = data_out->gyro_y / (SENSITIVITY_GYRO_Y);
    data_out->gyro_z = data_out->gyro_z / (SENSITIVITY_GYRO_Z);
    
    // Convert temperatures and calculate averages
    data_out->temp_due = GET_TEMPERATURE(data_in->temp_due_lsb);
    data_out->temp_uno = GET_TEMPERATURE(data_in->temp_uno_lsb);
}

/*!
    \brief Calculate cross-axis compensation

    \param data pointer to sensor data
*/
void scha63x_cross_axis_compensation(scha63x_real_data *data)
{
    float acc_x_comp, acc_y_comp, acc_z_comp;
    float gyro_x_comp, gyro_y_comp, gyro_z_comp;
        
    acc_x_comp = (scha63x_cac_values.bxx * data->acc_x) + (scha63x_cac_values.bxy * data->acc_y) + (scha63x_cac_values.bxz * data->acc_z);
    acc_y_comp = (scha63x_cac_values.byx * data->acc_x) + (scha63x_cac_values.byy * data->acc_y) + (scha63x_cac_values.byz * data->acc_z);
    acc_z_comp = (scha63x_cac_values.bzx * data->acc_x) + (scha63x_cac_values.bzy * data->acc_y) + (scha63x_cac_values.bzz * data->acc_z);
    gyro_x_comp = (scha63x_cac_values.cxx * data->gyro_x) + (scha63x_cac_values.cxy * data->gyro_y) + (scha63x_cac_values.cxz * data->gyro_z);
    gyro_y_comp = (scha63x_cac_values.cyx * data->gyro_x) + (scha63x_cac_values.cyy * data->gyro_y) + (scha63x_cac_values.cyz * data->gyro_z);
    gyro_z_comp = (scha63x_cac_values.czx * data->gyro_x) + (scha63x_cac_values.czy * data->gyro_y) + (scha63x_cac_values.czz * data->gyro_z);
    
    data->acc_x = acc_x_comp;
    data->acc_y = acc_y_comp;
    data->acc_z = acc_z_comp;
    data->gyro_x = gyro_x_comp;
    data->gyro_y = gyro_y_comp;
    data->gyro_z = gyro_z_comp;
}
