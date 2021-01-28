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
static uint32_t si7021_i2c_rx_data;
static uint32_t si7021_i2c_tx_data;

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
 * 	To switch to a different I2C peripheral, change the SI7021_I2C value defined in the header file
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
 * @param[in] byte_length
 * 	amount of bytes for operation
 *
 * @param[in] register_address
 * 	Register to be accessed in i2c operation
 *
 ******************************************************************************/
void si7021_read(uint32_t event_cb, uint32_t byte_length, uint32_t register_address){
	i2c_start(SI7021_I2C, SI7021_DEVICE_ADDRESS, register_address, READ, &si7021_i2c_rx_data, &si7021_i2c_tx_data, byte_length, event_cb);
}

/***************************************************************************//**
 * @brief
 *	Starts an i2c write to the si7021
 *
 * @details
 *	This function will start i2c communication between the master (PG12) and the SI7021.
 *	This function is setup for write operations
 *
 * @param[in] event_cb
 * 	event to be added to the scheduler upon completion of i2c operation
 *
 * @param[in] byte_length
 * 	amount of bytes for operation
 *
 * @param[in] register_address
 * 	Register to be accessed in i2c operation
 *
 ******************************************************************************/
void si7021_write(uint32_t event_cb, uint32_t byte_length, uint32_t register_address){
	i2c_start(SI7021_I2C, SI7021_DEVICE_ADDRESS, register_address, WRITE, &si7021_i2c_rx_data, &si7021_i2c_tx_data, byte_length, event_cb);
}

/***************************************************************************//**
 * @brief
 *	Converts read data to temperature in
 *
 * @details
 *	This function will convert the private si7021_i2c_rx_data variable passed into i2c_start
 *	function into a fahrenheit value. It uses the algorithm mentioned in the si7021's data
 *	sheet to convert into celsius first. That value is then converted into fahrenheit.
 *
 ******************************************************************************/
float si7021_temperatureF(){
	float celsius = ((175.72 * (float)si7021_i2c_rx_data) / 65536.0) - 46.85;
	float fahrenheit = celsius * (9.0 / 5.0) + 32.0;
	fahrenheit = (float)((int)(fahrenheit*10))/10;

 	return fahrenheit;
}

/***************************************************************************//**
 * @brief
 *	Converts read data to relative humidity percentage
 *
 * @details
 *	This function will convert the private si7021_i2c_rx_data variable passed into i2c_start
 *	function into a RH percentage. It uses the algorithm mentioned in the si7021's data
 *	sheet to convert into a proper percentage.
 *
 ******************************************************************************/
float si7021_rh(){
	float rh = ((125.0 * (float)si7021_i2c_rx_data) / 65536.0) - 6.0;
	rh = (float)((int)(rh*10))/10;

 	return rh;
}

/***************************************************************************//**
 * @brief
 *	SI7021 test will test out the functionality of multiple and single byte operations of I2C Serial
 *	Communication. It tests by reading and writing multiple and single bits of the user register 1 and
 *	the temperature reading functionality of the SI7021.
 *
 * @details
 *	This function will access both the temperature and user 1 registers to perform both read and write operations.
 *	First, it will read user register 1 to verify that it is set in it's default state at boot up. It will then change
 *	resolution of the temperature read register to 13 bit from 12 bit. Lastly, it will then ensure that the temperature
 *	read operation still functions.
 *
 *	@return
 *   Returns bool true if successfully passed through the TDD tests in this
 *   function
 ******************************************************************************/
bool si7021_test(){
	// Do hardware delay first (80 ms)
	timer_delay(80);

	// This first test verifies functionality of single byte reads
	// It reads from user register 1
	i2c_start(SI7021_I2C, SI7021_DEVICE_ADDRESS, SI7021_UR1_READ_CMD, READ, &si7021_i2c_rx_data, &si7021_i2c_tx_data, ONE_BYTE, 0); // sets event input to 0 as to not test event implementation
	while(i2c_busy());
	// Validate the read
	uint32_t read_value = 0x3A;
	timer_delay(80);
	EFM_ASSERT(read_value == si7021_i2c_rx_data || si7021_i2c_rx_data == 0xBA); // checks for both possible resolutions

	// This next test tests the functionality of writes
	// It will write to user register 1 to change the resolution to 13 bit
	si7021_i2c_tx_data = 0xBA;
	si7021_write(0, ONE_BYTE, SI7021_UR1_WRITE_CMD); // verify that new write function works. Same function as line below
//	i2c_start(SI7021_I2C, SI7021_DEVICE_ADDRESS, SI7021_UR1_WRITE_CMD, WRITE, &si7021_i2c_rx_data, &si7021_i2c_tx_data, ONE_BYTE, 0);
	while(i2c_busy());
	timer_delay(80);
	i2c_start(SI7021_I2C, SI7021_DEVICE_ADDRESS, SI7021_UR1_READ_CMD, READ, &si7021_i2c_rx_data, &si7021_i2c_tx_data, ONE_BYTE, 0);
	read_value = 0xBA;
	while(i2c_busy());
	// Validate the change in resolution
	EFM_ASSERT(read_value == si7021_i2c_rx_data);

	// This tests the functionality of the temperature reads
	i2c_start(SI7021_I2C, SI7021_DEVICE_ADDRESS, SI7021_READ_TEMP_CMD, READ, &si7021_i2c_rx_data, &si7021_i2c_tx_data, TWO_BYTES, 0);
	// Validate correct temp read (in range of 60-85 degrees)
	while(i2c_busy()); // stall until i2c is done
	float temperature = si7021_temperatureF();	// get data and compare
	EFM_ASSERT((temperature > 60.0) && (temperature < 85.0));

	return true;
}
