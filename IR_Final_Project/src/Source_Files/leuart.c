/**
 * @file leuart.c
 * @author Ivan Rodriguez
 * @date   October 20, 2020
 * @brief Contains all the functions of the LEUART peripheral
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Library includes
#include <string.h>

//** Silicon Labs include files
#include "em_gpio.h"
#include "em_cmu.h"

//** Developer/user include files
#include "leuart.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************
uint32_t	rx_done_evt;
uint32_t	tx_done_evt;
bool		leuart0_tx_busy;
static LEUART_STATE_MACHINE leuart_cmd;

/***************************************************************************//**
 * @brief LEUART driver
 * @details
 *  This module contains all the functions to support the driver's state
 *  machine to transmit a string of data across the LEUART bus.  There are
 *  additional functions to support the Test Driven Development test that
 *  is used to validate the basic set up of the LEUART peripheral.  The
 *  TDD test for this class assumes that the LEUART is connected to the HM-18
 *  BLE module.  These TDD support functions could be used for any TDD test
 *  to validate the correct setup of the LEUART.
 *
 ******************************************************************************/

//***********************************************************************************
// Private functions
//***********************************************************************************
static void leuart_txbl(void);
static void leuart_txc(void);

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Opens  LEUART peripheral
 *
 * @details
 * 	This function will open a LEUART peripheral, and setup according to the open struct.
 * 	It will also route the RX and TX pins and enable interrupts at the CPU level.
 *
 * @param[in] leuart
 * 	pointer to the leuart peripheral to be used in this setup
 *
 * @param [in]leuart_settings
 * 	pointer to the LEUART open struct that will be used to initialize the LEUART peripheral
 *
 ******************************************************************************/

void leuart_open(LEUART_TypeDef *leuart, LEUART_OPEN_STRUCT *leuart_settings){

	// Enable clock to peripheral
	if (leuart == LEUART0)
		CMU_ClockEnable(cmuClock_LEUART0, true);

	// Clock check
	if (leuart->STARTFRAME == 0x00){
		leuart->STARTFRAME = 0x01;
		while(leuart->SYNCBUSY);
		EFM_ASSERT(leuart->STARTFRAME == 0x01);
	}

	else{
		leuart->STARTFRAME = 0x00;
		while(leuart->SYNCBUSY);
		EFM_ASSERT(leuart->STARTFRAME == 0x00);
	}

	// Peripheral initialization

	LEUART_Init_TypeDef leuart_setup;

	leuart_setup.baudrate = leuart_settings->baudrate;
	leuart_setup.databits = leuart_settings->databits;
	leuart_setup.enable = leuart_settings->enable;
	leuart_setup.parity = leuart_settings->parity;
	leuart_setup.refFreq = leuart_settings->refFreq;
	leuart_setup.stopbits = leuart_settings->stopbits;

	LEUART_Init(leuart, &leuart_setup);
	while(leuart->SYNCBUSY);

	// Pin Route
	leuart->ROUTEPEN = (LEUART_ROUTEPEN_RXPEN * leuart_settings->rx_pin_en) | (LEUART_ROUTEPEN_TXPEN * leuart_settings->tx_pin_en);
	leuart->ROUTELOC0 = leuart_settings->rx_loc | leuart_settings->tx_loc;

	// Clear RX and TX buffers
	while(leuart->SYNCBUSY);
	leuart->CMD |= LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX;

	LEUART_Enable(leuart, leuart_settings->enable);
	//while(leuart->SYNCBUSY);
	//leuart->CMD |= leuart_settings->tx_en | leuart_settings->rx_en; // does the same as above(Enable).

	EFM_ASSERT((leuart->STATUS & LEUART_STATUS_RXENS) & (leuart_settings->rx_en * LEUART_STATUS_RXENS));
	EFM_ASSERT((leuart->STATUS & LEUART_STATUS_TXENS) & (leuart_settings->tx_en * LEUART_STATUS_TXENS));

	// Enable interrupts (clear first)
	leuart->IFC |= _LEUART_IFC_MASK;
	//leuart->IEN |= LEUART_IEN_TXBL | LEUART_IEN_TXC; // can't enable in open,
	//														must enable TXBL in start fcn. TXC once string is completely sent

	// CPU Interrupt Enable
	if (leuart == LEUART0)
		NVIC_EnableIRQ(LEUART0_IRQn);

	// set up rx/tx events
	rx_done_evt = leuart_settings->rx_done_evt;
	tx_done_evt = leuart_settings->tx_done_evt;
	leuart_cmd.current_state = INITIALIZATION;
}

/***************************************************************************//**
 * @brief
 *	LEUART0 peripheral interrupt handler
 *
 * @details
 *	This function handles all interrupts configured for LEUART operation configured by the user. Currently configured for
 *	TXBL and TXC interrupts
 *
 ******************************************************************************/

void LEUART0_IRQHandler(void){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	uint32_t int_flag = LEUART0->IF & LEUART0->IEN;
	LEUART0->IFC = int_flag;

	if (int_flag & LEUART_IEN_TXBL){
		leuart_txbl();
	}
	if (int_flag & LEUART_IEN_TXC){
		leuart_txc();
	}

	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *	TXBL interrupt function
 *
 * @details
 *	This function will be called by the LEAURT interrupt handler on an TXBL interrupt
 *	and act based upon the configured state machine
 *
 ******************************************************************************/
void leuart_txbl(){
	switch(leuart_cmd.current_state){
		case INITIALIZATION:
			EFM_ASSERT(false);
			break;

		case DATA_TRANSMISSION:
			if (leuart_cmd.string_count < leuart_cmd.string_len){
				leuart_cmd.leaurt->TXDATA = *leuart_cmd.string;
				leuart_cmd.string++;
				leuart_cmd.string_count++;
			}

			else{
				leuart_cmd.leaurt->IEN &= ~LEUART_IEN_TXBL;
				leuart_cmd.leaurt->IEN |= LEUART_IEN_TXC;
				leuart_cmd.current_state = CLOSE;
			}
			break;

		case CLOSE:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

/***************************************************************************//**
 * @brief
 *	TXC interrupt function
 *
 * @details
 *	This function will be called by the LEAURT interrupt handler on an TXC interrupt
 *	and act based upon the configured state machine
 *
 ******************************************************************************/
void leuart_txc(){
	switch(leuart_cmd.current_state){
			case INITIALIZATION:
				EFM_ASSERT(false);
				break;

			case DATA_TRANSMISSION:
				EFM_ASSERT(false);
				break;

			case CLOSE:
				leuart_cmd.leaurt->IEN &= ~LEUART_IEN_TXC & ~LEUART_IEN_TXBL ;
				leuart0_tx_busy = false;
				add_scheduled_event(tx_done_evt);
				sleep_unblock_mode(LEUART_TX_EM);
				leuart_cmd.current_state = INITIALIZATION;
				break;
			default:
				EFM_ASSERT(false);
				break;
		}
}

/***************************************************************************//**
 * @brief
 * 	Begins LEUART transmission
 *
 * @details
 * 	This functions starts a transmission and sets the string to be transmitted on the peripheral.
 * 	It sets the state machine and enables the TXBL interrupt to enable transmission.
 *
 * @param[in] leuart
 * 	Pointer to the LEAURT peripheral to be used in the operation
 *
 * @param[in]string
 * 	Pointer to the string to be transmitted
 *
 * @param[in] string_len
 * 	The length of the string being transmitted
 *
 ******************************************************************************/
void leuart_start(LEUART_TypeDef *leuart, char *string, uint32_t string_len){
	while(!(leuart->STATUS & LEUART_STATUS_TXIDLE));

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	sleep_block_mode(LEUART_TX_EM);
	leuart_cmd.leaurt = leuart;
	leuart_cmd.string = string;
	leuart_cmd.string_len = string_len;
	leuart_cmd.string_count = 0;
	leuart0_tx_busy = true;
	leuart_cmd.current_state = DATA_TRANSMISSION;

	leuart_cmd.leaurt->IEN |= LEUART_IEN_TXBL;

	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 * 	State of the leuart state machine
 *
 * @details
 *	This function is used to access the private variable of leuart0_tx_busy.
 *	Currently only supports LEUART0
 *
 * @param[in] leuart
 * 	leuart peripheral to be used
 *
 ******************************************************************************/

bool leuart_tx_busy(LEUART_TypeDef *leuart){
	return leuart0_tx_busy;
}

/***************************************************************************//**
 * @brief
 *   LEUART STATUS function returns the STATUS of the peripheral for the
 *   TDD test
 *
 * @details
 * 	 This function enables the LEUART STATUS register to be provided to
 * 	 a function outside this .c module.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the STATUS register value as an uint32_t value
 *
 ******************************************************************************/

uint32_t leuart_status(LEUART_TypeDef *leuart){
	uint32_t	status_reg;
	status_reg = leuart->STATUS;
	return status_reg;
}

/***************************************************************************//**
 * @brief
 *   LEUART CMD Write sends a command to the CMD register
 *
 * @details
 * 	 This function is used by the TDD test function to program the LEUART
 * 	 for the TDD tests.
 *
 * @note
 *   Before exiting this function to update  the CMD register, it must
 *   perform a SYNCBUSY while loop to ensure that the CMD has by synchronized
 *   to the lower frequency LEUART domain.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] cmd_update
 * 	 The value to write into the CMD register
 *
 ******************************************************************************/

void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update){

	leuart->CMD = cmd_update;
	while(leuart->SYNCBUSY);
}

/***************************************************************************//**
 * @brief
 *   LEUART IF Reset resets all interrupt flag bits that can be cleared
 *   through the Interrupt Flag Clear register
 *
 * @details
 * 	 This function is used by the TDD test program to clear interrupts before
 * 	 the TDD tests and to reset the LEUART interrupts before the TDD
 * 	 exits
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 ******************************************************************************/

void leuart_if_reset(LEUART_TypeDef *leuart){
	leuart->IFC = 0xffffffff;
}

/***************************************************************************//**
 * @brief
 *   LEUART App Transmit Byte transmits a byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a transmit byte, a while statement checking for the TXBL
 *   bit in the Interrupt Flag register is required before writing the
 *   TXDATA register.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] data_out
 *   Byte to be transmitted by the LEUART peripheral
 *
 ******************************************************************************/

void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out){
	while (!(leuart->IF & LEUART_IF_TXBL));
	leuart->TXDATA = data_out;
}


/***************************************************************************//**
 * @brief
 *   LEUART App Receive Byte polls a receive byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a receive byte, a while statement checking for the RXDATAV
 *   bit in the Interrupt Flag register is required before reading the
 *   RXDATA register.
 *
 * @param[in] leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the byte read from the LEUART peripheral
 *
 ******************************************************************************/

uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart){
	uint8_t leuart_data;
	while (!(leuart->IF & LEUART_IF_RXDATAV));
	leuart_data = leuart->RXDATA;
	return leuart_data;
}
