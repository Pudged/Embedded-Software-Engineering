/**
 * @file i2c.c
 * @author Ivan Rodriguez
 * @date September 29th, 2020
 * @brief I2C file
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "i2c.h"

//***********************************************************************************
// Private variables
//***********************************************************************************
static I2C_STATE_MACHINE i2c_cmd;

//***********************************************************************************
// Private functions
//***********************************************************************************
static void i2c_bus_reset(I2C_TypeDef *i2c);
static void i2c_ack();
static void i2c_nack();
static void i2c_rxdatav();
static void i2c_mstop();

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Opens an i2c peripheral and starts the bus
 *
 * @details
 *	This function will begin an i2c serial communication bus and enable all related interrupts
 *	(ACK, NACK, RXDATAV, MSTOP)
 *
 * @note
 * 	This function will start either i2c peripheral (I2C0, I2C1) depending on the TypeDef passed in.
 *
 * @param[in] i2c
 *	This will be the i2c peripheral being used (I2C0 or I2C1)
 *
 * @param[in] i2c_setup
 * 	setup struct used to configure the i2c bus
 *
 ******************************************************************************/
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup){
	I2C_Init_TypeDef i2c_values;
	if (i2c == I2C0)
		CMU_ClockEnable(cmuClock_I2C0, true);
	else if (i2c == I2C1)
		CMU_ClockEnable(cmuClock_I2C1, true);

	if ((i2c->IF & 0x01)){
		i2c->IFS = 0x01;
		EFM_ASSERT(i2c->IF & 0x01);
		i2c->IFC = 0x01;
	}
	else{
		i2c->IFC = 0x01;
		EFM_ASSERT(!(i2c->IF &0x01));
	}

	// Initialization of I2C
	i2c_values.enable = i2c_setup->enable;
	i2c_values.master = i2c_setup->master;
	i2c_values.refFreq = i2c_setup->refFreq;
	i2c_values.freq = i2c_setup->freq;
	i2c_values.clhr = i2c_setup->clhr;

	I2C_Init(i2c, &i2c_values);

	// GPIO Pin Enable and Routing
	i2c->ROUTEPEN = (I2C_ROUTEPEN_SCLPEN * i2c_setup->scl_en) | (I2C_ROUTEPEN_SDAPEN * i2c_setup->sda_en);
	i2c->ROUTELOC0 = i2c_setup->scl_route | i2c_setup->sda_route;

	// Enabling Interrupts (Clear first)
	i2c->IFC |= _I2C_IFC_MASK;
	i2c->IEN |= I2C_IEN_ACK | I2C_IEN_NACK | I2C_IEN_RXDATAV | I2C_IEN_MSTOP;

	// CPU interrupt enable
	if (i2c == I2C0)
		NVIC_EnableIRQ(I2C0_IRQn);
	else if (i2c == I2C1)
		NVIC_EnableIRQ(I2C1_IRQn);

	i2c_bus_reset(i2c);
}

/***************************************************************************//**
 * @brief
 *	Resets i2c bus
 *
 * @details
 *	This function will reset the i2c bus by clearing the STATE register of the i2c peripheral
 *
 * @note
 * 	This functions will reset either i2c peripheral (I2C0, I2C1) depending on the TypeDef passed in.
 *
 * @param[in] i2c
 *	This will be the i2c peripheral being used (I2C0 or I2C1)
 *
 ******************************************************************************/
void i2c_bus_reset(I2C_TypeDef *i2c){
	if(i2c->STATE & I2C_STATE_BUSY){
		i2c->CMD = I2C_CMD_ABORT;
		while(i2c->STATE & I2C_STATE_BUSY); // wait for the Abort function to clear the busy bit
	}

	uint32_t save_state = i2c->IEN;
	i2c->IEN &= ~_I2C_IEN_MASK;
	i2c->IFC |= _I2C_IFC_MASK;
	i2c->CMD |= I2C_CMD_CLEARTX;
	i2c->CMD |= I2C_CMD_START | I2C_CMD_STOP;

	while(!(i2c->IF & I2C_IF_MSTOP)); // stall until STOP is completed

	i2c->IFC |= _I2C_IFC_MASK;
	i2c->IEN = save_state;
	i2c->CMD |= I2C_CMD_ABORT;
}

/***************************************************************************//**
 * @brief
 *	Starts i2c communication
 *
 * @details
 *	This function will begin i2c serial communication between two devices. It supports both reading and writing
 *
 * @note
 * 	This functions will only function while the i2c peripheral is in the IDLE state
 *
 * @param[in] i2c
 *	This will be the i2c peripheral being used (I2C0 or I2C1)
 *
 * @param[in] device_address
 * 	address of the slave device
 *
 * @param[in] register_address
 *  address for register to write to or command for read operation
 *
 * @param[in] rw
 *  boolean value for read or write operation
 *
 * @param[in] rx_data
 * 	pointer to store data from read operation
 *
 * @param[in] tx_data
 * 	pointer to store data for write operation
 *
 * @param[in] num_bytes
 * 	length of bytes for operation
 *
 * @param[in] event_cb
 * 	event callback to add to scheduled events once i2c operation comes to an end
 *
 ******************************************************************************/
void i2c_start(I2C_TypeDef *i2c, uint32_t device_address, uint32_t register_address, bool rw, uint32_t *rx_data, uint32_t *tx_data, uint32_t num_bytes, uint32_t event_cb){
	EFM_ASSERT((i2c->STATE & _I2C_STATE_STATE_MASK) == I2C_STATE_STATE_IDLE);
	i2c_cmd.sm_busy = true;
	sleep_block_mode(I2C_EM_BLOCK);

	i2c_cmd.i2c = i2c;
	i2c_cmd.device_address = device_address;
	i2c_cmd.register_address = register_address;
	i2c_cmd.rw = rw;
	i2c_cmd.tx_data = tx_data;
	i2c_cmd.rx_data = rx_data;
	i2c_cmd.num_bytes = num_bytes;
	i2c_cmd.event_cb = event_cb;

	i2c_cmd.current_state = SEND_DEVICE_ADDR;

	// Send start bit
	i2c_cmd.i2c->CMD = I2C_CMD_START;
	i2c_cmd.i2c->TXDATA = (i2c_cmd.device_address << 1) | WRITE;
}

/***************************************************************************//**
 * @brief
 *	Accesses the private SM busy variable from the static i2c_cmd struct.
 *
 * @details
 *	This function will return whether the state machine for I2C is busy or not.
 *
 *
 ******************************************************************************/
bool i2c_busy(void){
	return (i2c_cmd.sm_busy);
}

/***************************************************************************//**
 * @brief
 *	I2C0 peripheral interrupt handler
 *
 * @details
 *	This function handles all interrupts configured for i2c operation configured by the user. Currently configured for
 *	ACK, NACK, RXDATAV, and MSTOP interrupts
 *
 ******************************************************************************/
void I2C0_IRQHandler(void){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	uint32_t int_flag = I2C0->IF & I2C0->IEN;
	I2C0->IFC = int_flag;

	if(int_flag & I2C_IEN_ACK){
		i2c_ack();
	}
	if(int_flag & I2C_IEN_NACK){
		i2c_nack();
	}
	if(int_flag & I2C_IEN_RXDATAV){
		i2c_rxdatav();
	}
	if(int_flag & I2C_IEN_MSTOP){
		i2c_mstop();
	}

	CORE_EXIT_CRITICAL();
}


/***************************************************************************//**
 * @brief
 *	I2C1 peripheral interrupt handler
 *
 * @details
 *	This function handles all interrupts configured for i2c operation configured by the user. Currently configured for
 *	ACK, NACK, RXDATAV, and MSTOP interrupts
 *
 ******************************************************************************/
void I2C1_IRQHandler(void){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	uint32_t int_flag = I2C1->IF & I2C1->IEN;
	I2C1->IFC = int_flag;

	if(int_flag & I2C_IEN_ACK){
		i2c_ack();
	}
	if(int_flag & I2C_IEN_NACK){
		i2c_nack();
	}
	if(int_flag & I2C_IEN_RXDATAV){
		i2c_rxdatav();
	}
	if(int_flag & I2C_IEN_MSTOP){
		i2c_mstop();
	}

	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *	ACK interrupt function
 *
 * @details
 *	This function will be called by the i2c interrupt handler on an ACK interrupt and act based upon the configured state machine
 *
 ******************************************************************************/
void i2c_ack(){
	switch(i2c_cmd.current_state){
		case SEND_DEVICE_ADDR:
			i2c_cmd.i2c->TXDATA = i2c_cmd.register_address;
			i2c_cmd.current_state = SEND_REGISTER_ADDR;
			break;

		case SEND_REGISTER_ADDR:
			if (i2c_cmd.rw){
				i2c_cmd.current_state = WAIT_CONVERSION;
				i2c_cmd.i2c->CMD = I2C_CMD_START;
				i2c_cmd.i2c->TXDATA = (i2c_cmd.device_address << 1) | READ;
			}
			else{ // had to add the first byte write, else it sits in a loop since the slave waits for a byte to then send back an ACK
				i2c_cmd.num_bytes--;
				uint8_t tx_byte;
				tx_byte = (*(i2c_cmd.tx_data) << (SHIFT * i2c_cmd.num_bytes));
				i2c_cmd.i2c->TXDATA = tx_byte;
				i2c_cmd.current_state = DATA_WRITE;
			}
			break;

		case WAIT_CONVERSION:
			i2c_cmd.current_state = DATA_READ;
			break;

		case DATA_READ:
			EFM_ASSERT(false);
			break;

		case DATA_WRITE:
			if (i2c_cmd.num_bytes <= 0){
				i2c_cmd.current_state = SEND_STOP_CMD;
				i2c_cmd.i2c->CMD = I2C_CMD_STOP;
			}
			else{
				i2c_cmd.num_bytes--;
				uint8_t tx_byte;
				tx_byte = (*(i2c_cmd.tx_data) << (SHIFT * i2c_cmd.num_bytes));
				i2c_cmd.i2c->TXDATA = tx_byte;
			}



			break;

		case SEND_STOP_CMD:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

/***************************************************************************//**
 * @brief
 *	NACK interrupt function
 *
 * @details
 *	This function will be called by the i2c interrupt handler on an NACK interrupt and act based upon the configured state machine
 *
 ******************************************************************************/
void i2c_nack(){
	switch(i2c_cmd.current_state){
		case SEND_DEVICE_ADDR:
			EFM_ASSERT(false);
			break;

		case SEND_REGISTER_ADDR:
			EFM_ASSERT(false);
			break;

		case WAIT_CONVERSION:
			i2c_cmd.i2c->CMD = I2C_CMD_START;
			i2c_cmd.i2c->TXDATA = (i2c_cmd.device_address << 1) | READ;
			break;

		case DATA_READ:
			EFM_ASSERT(false);
			break;

		case SEND_STOP_CMD:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

/***************************************************************************//**
 * @brief
 *	RXDATAV interrupt function
 *
 * @details
 *	This function will be called by the i2c interrupt handler on an RXDATAV interrupt and act based upon the configured state machine
 *
 ******************************************************************************/
void i2c_rxdatav(){
	switch(i2c_cmd.current_state){
		case SEND_DEVICE_ADDR:
			EFM_ASSERT(false);
			break;

		case SEND_REGISTER_ADDR:
			EFM_ASSERT(false);
			break;

		case WAIT_CONVERSION:
			EFM_ASSERT(false);
			break;

		case DATA_READ:
			i2c_cmd.num_bytes--;
			*(i2c_cmd.rx_data) &= ~(0xff << (SHIFT * i2c_cmd.num_bytes));
			*(i2c_cmd.rx_data) |= (i2c_cmd.i2c->RXDATA << (SHIFT * i2c_cmd.num_bytes));

			if(i2c_cmd.num_bytes > 0){
				i2c_cmd.i2c->CMD = I2C_CMD_ACK;
			}

			else{
				i2c_cmd.i2c->CMD = I2C_CMD_NACK;
				i2c_cmd.i2c->CMD = I2C_CMD_STOP;
				i2c_cmd.current_state = SEND_STOP_CMD;
			}

			break;

		case DATA_WRITE:
			EFM_ASSERT(false);
			break;
		case SEND_STOP_CMD:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

/***************************************************************************//**
 * @brief
 *	ACK interrupt function
 *
 * @details
 *	This function will be called by the i2c interrupt handler on an MSTOP interrupt and act based upon the configured state machine
 *
 * @note
 *	An event will be added to the scheduler upon completion of this function
 *
 ******************************************************************************/
void i2c_mstop(){
	switch(i2c_cmd.current_state){
		case SEND_DEVICE_ADDR:
			EFM_ASSERT(false);
			break;

		case SEND_REGISTER_ADDR:
			EFM_ASSERT(false);
			break;

		case WAIT_CONVERSION:
			EFM_ASSERT(false);
			break;

		case DATA_READ:
			EFM_ASSERT(false);
			break;

		case DATA_WRITE:
			EFM_ASSERT(false);
			break;

		case SEND_STOP_CMD:
			i2c_cmd.current_state = SEND_DEVICE_ADDR;
			i2c_cmd.sm_busy = false;
			sleep_unblock_mode(I2C_EM_BLOCK);
			add_scheduled_event(i2c_cmd.event_cb);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

