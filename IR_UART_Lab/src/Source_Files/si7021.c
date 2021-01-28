/**
 * @file si7021.c
 * @author Ivan Rodriguez
 * @date October 2nd, 2020
 * @brief si7021 implementation file
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "si7021.h"

//***********************************************************************************
// Private variables
//***********************************************************************************
static uint16_t si7021_i2c_data;

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Opens si7021 i2c peripheral
 *
 * @details
 *	This function will setup the si7021 for i2c communication
 *
 * @note
 * 	To switch to a different I2C peripheral, change teh SI7021_I2C value defined in the header file
 *
 ******************************************************************************/
void si7021_i2c_open(){
	I2C_OPEN_STRUCT si7021_open;
	si7021_open.enable = true;
	si7021_open.master = 	true;
	si7021_open.refFreq = 	SI7021_REF_FREQ;
	si7021_open.freq = 		SI7021_FREQ;
	si7021_open.clhr = 		SI7021_CLHR;
	si7021_open.scl_route = SI7021_SCL_ROUTE;
	si7021_open.sda_route = SI7021_SDA_ROUTE;
	si7021_open.scl_en = 	SI7021_SCL_EN;
	si7021_open.sda_en = 	SI7021_SDA_EN;

	i2c_open(SI7021_I2C, &si7021_open);
}

/***************************************************************************//**
 * @brief
 *	Starts an i2c read from the si7021
 *
 * @details
 *	This function will start i2c communication between the master (PG12) and the SI7021.
 *	This function is setup for read operations
 *
 * @param[in] event_cb
 * 	event to be added to the scheduler upon completion of i2c operation
 *
 ******************************************************************************/
void si7021_read(uint32_t event_cb){
	i2c_start(SI7021_I2C, SI7021_DEVICE_ADDRESS, SI7021_READ_CMD, READ, &si7021_i2c_data, SI7021_READ_CB);
}

/***************************************************************************//**
 * @brief
 *	Converts read data to temperature in
 *
 * @details
 *	This function will convert the private si7021_i2c_data variable passed into i2c_start
 *	function into a fahrenheit value. It uses the algorithm mentioned in the si7021's data
 *	sheet to convert into celsius first. That value is then converted into fahrenheit.
 *
 ******************************************************************************/
float si7021_temperatureF(){
	float celsius = ((175.72 * (float)si7021_i2c_data) / 65536.0) - 46.85;
	float fahrenheit = celsius * (9.0 / 5.0) + 32.0;
	fahrenheit = (float)((int)(fahrenheit*10))/10;

 	return fahrenheit;
}
