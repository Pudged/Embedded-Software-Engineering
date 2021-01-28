#ifndef SRC_SOURCE_FILES_I2C_H_
#define SRC_SOURCE_FILES_I2C_H_

/* Silicon Labs include statements */
#include "em_i2c.h"
#include "em_cmu.h"

/* User include statements */
#include "sleep_routines.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************
#define I2C_EM_BLOCK 		EM2
#define WRITE				0
#define READ				1
#define SHIFT				0x8UL
#define ONE_BYTE			1
#define TWO_BYTES			2

//***********************************************************************************
// global variables
//***********************************************************************************
typedef struct {
	bool 					enable;		// enable i2c upon completion of open
	bool					master;		// master (true) or slave (false)
	uint32_t 				refFreq;	// i2c reference clock
	uint32_t 				freq;		// i2c bus frequency
	I2C_ClockHLR_TypeDef 	clhr;		// clock low/high ratio control
	uint32_t				scl_route;	// route to scl pin
	uint32_t				sda_route;	// route to sda pin
	bool					scl_en;		// enable scl pin
	bool					sda_en;		// enable sda pin
} I2C_OPEN_STRUCT;

typedef enum {
	SEND_DEVICE_ADDR,
	SEND_REGISTER_ADDR,
	WAIT_CONVERSION,
	DATA_READ,
	DATA_WRITE,
	SEND_STOP_CMD
} i2c_states;

typedef struct {
	i2c_states			current_state;
	I2C_TypeDef 		*i2c;
	uint32_t 			device_address;
	uint32_t 			register_address;
	bool 				rw;
	uint32_t 			*rx_data;
	uint32_t			*tx_data;
	uint32_t 			num_bytes;
	volatile bool 		sm_busy;
	uint32_t 			event_cb;

} I2C_STATE_MACHINE;

//***********************************************************************************
// function prototypes
//***********************************************************************************
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup);
void i2c_start(I2C_TypeDef *i2c, uint32_t device_address, uint32_t register_address, bool rw, uint32_t *rx_data, uint32_t *tx_data, uint32_t num_bytes, uint32_t event_cb);
void I2C0_IRQHandler(void);
bool i2c_busy(void);
#endif /* SRC_SOURCE_FILES_I2C_H_ */
