#ifndef SRC_HEADER_FILES_SI7021_H_
#define SRC_HEADER_FILES_SI7021_H_

/* Include statements */
#include "i2c.h"
#include "HW_delay.h"

//***********************************************************************************
// defined files
//***********************************************************************************
#define	SI7021_FREQ				I2C_FREQ_STANDARD_MAX
#define SI7021_REF_FREQ			0
#define SI7021_CLHR				i2cClockHLRStandard
#define SI7021_SCL_EN			true
#define SI7021_SDA_EN			true
//created below macros to allow use of either I2C0 or I2C1
#define SI7021_I2C_N			0
#if SI7021_I2C_N == 0
	#define SI7021_I2C			I2C0
	#define SI7021_SCL_ROUTE	I2C_ROUTELOC0_SCLLOC_LOC15
	#define SI7021_SDA_ROUTE	I2C_ROUTELOC0_SDALOC_LOC15
#elif SI7021_I2C_N == 1
	#define SI7021_I2C			I2C1
	#define SI7021_SCL_ROUTE	I2C_ROUTELOC0_SCLLOC_LOC19
	#define SI7021_SDA_ROUTE	I2C_ROUTELOC0_SDALOC_LOC19
#endif
#define SI7021_DEVICE_ADDRESS	0x40
#define SI7021_READ_TEMP_CMD	0xF3
#define SI7021_READ_RH_CMD		0xF5

// TDD
#define SI7021_UR1_READ_CMD		0xE7
#define SI7021_UR1_WRITE_CMD	0xE6

//***********************************************************************************
// function prototypes
//***********************************************************************************
void si7021_i2c_open(void);
void si7021_read(uint32_t event_cb, uint32_t byte_length, uint32_t register_address);
void si7021_write(uint32_t event_cb, uint32_t byte_length, uint32_t register_address);
float si7021_temperatureF(void);
float si7021_rh(void);
bool si7021_test();

#endif /* SRC_HEADER_FILES_SI7021_H_ */
