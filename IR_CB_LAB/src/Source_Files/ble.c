/**
 * @file ble.c
 * @author Ivan Rodriguez/ Professor Graham
 * @date October 22,2020
 * @brief Contains all the functions to interface the application with the HM-18
 *   BLE module and the LEUART driver
 *
 */


//***********************************************************************************
// Include files
//***********************************************************************************
#include "ble.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************
static CIRC_TEST_STRUCT test_struct;
static BLE_CIRCULAR_BUF ble_cbuf;
static char popped_string[CSIZE];

/***************************************************************************//**
 * @brief BLE module
 * @details
 *  This module contains all the functions to interface the application layer
 *  with the HM-18 Bluetooth module.  The application does not have the
 *  responsibility of knowing the physical resources required, how to
 *  configure, or interface to the Bluetooth resource including the LEUART
 *  driver that communicates with the HM-18 BLE module.
 *
 ******************************************************************************/

//***********************************************************************************
// Private functions
//***********************************************************************************
static void ble_circ_init(void);
static void ble_circ_push(char *string);
static uint8_t ble_circ_space(void);
static void update_circ_wrtindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by);
static void update_circ_readindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by);

//***********************************************************************************
// Global functions
//***********************************************************************************


/***************************************************************************//**
 * @brief
 *	Opens the HM-10's functionality with the specified LEAURT peripheral
 *
 * @details
 * 	This function creates the open struct based on the HM-10's specifications which are setup in the header file
 * 	macros. It then uses these settings to open up communication on the LEUART peripheral
 *
 * @param[in] tx_event
 * 	Event callback for the TX done event
 *
 * @param[in] rx_event
 * 	Event callback for the RX done event
 *
 ******************************************************************************/

void ble_open(uint32_t tx_event, uint32_t rx_event){
	LEUART_OPEN_STRUCT leuart_setup_struct;

	leuart_setup_struct.baudrate = HM10_BAUDRATE;
	leuart_setup_struct.databits = HM10_DATABITS;
	leuart_setup_struct.enable = HM10_ENABLE;
	leuart_setup_struct.parity = HM10_PARITY;
	leuart_setup_struct.refFreq = HM10_REFFREQ;
	leuart_setup_struct.stopbits = HM10_STOPBITS;
	leuart_setup_struct.rx_done_evt = rx_event;
	leuart_setup_struct.rx_en = true;
	leuart_setup_struct.rx_loc = LEUART0_RX_ROUTE;
	leuart_setup_struct.rx_pin_en = true;
	leuart_setup_struct.tx_done_evt = tx_event;
	leuart_setup_struct.tx_en = true;
	leuart_setup_struct.tx_loc = LEUART0_TX_ROUTE;
	leuart_setup_struct.tx_pin_en = true;

	leuart_open(HM10_LEUART0, &leuart_setup_struct);
	ble_circ_init();
}


/***************************************************************************//**
 * @brief
 * 	Writes string to LEUART peripheral
 *
 * @details
 * 	This function begins the LEUART transmission for the HM-10 BLE module and sends a
 * 	string to it.
 *
 * @param[in] string
 * 	string to be transmitted over LEUART to the BLE module
 *
 ******************************************************************************/

void ble_write(char* string){
//	leuart_start(HM10_LEUART0, string, strlen(string));

	ble_circ_push(string);
	ble_circ_pop(false);
}

/***************************************************************************//**
 * @brief
 * 	Pops string from the circular buffer
 *
 * @details
 * 	This function pops a string off the circular buffer and passes the string to either the TDD
 * 	circular buffer test or the LEUART0 to transmit to the HM10 BLE Module
 *
 * @param[in] test
 *  boolean value used to decide between circular buffer test or LEUART transmission
 *
 * @return
 *  returns true if the buffer is empty and false if not empty
 ******************************************************************************/
bool ble_circ_pop(bool test){
	// check if empty
	if (ble_cbuf.read_ptr == ble_cbuf.write_ptr){
		return true;
	}

	// check if leuart is busy
	if (leuart_tx_busy(HM10_LEUART0)){
			return false;
	}

	else{
		uint8_t string_len = ble_cbuf.cbuf[ble_cbuf.read_ptr] - 1;

		update_circ_readindex(&ble_cbuf, 1);

		for(int i = 0; i < string_len; i++){
			popped_string[i] = ble_cbuf.cbuf[ble_cbuf.read_ptr];
			update_circ_readindex(&ble_cbuf, 1);
		}

		popped_string[string_len] = '\0'; // null

		if(test){
			for (int i = 0; i <= string_len; i++){
				test_struct.result_str[i] = popped_string[i];
			}
		}

		else{
			leuart_start(HM10_LEUART0, popped_string, string_len);
		}

		return false;
	}

	return true;
}

/***************************************************************************//**
 * @brief
 * 	Initializes the circular buffer
 *
 * @details
 * 	This function sets up the circular buffer used for transmission of multiple strings
 * 	to the LEUART
 *
 ******************************************************************************/
void ble_circ_init(void){
	ble_cbuf.read_ptr = 0;
	ble_cbuf.write_ptr = 0;
	ble_cbuf.size = CSIZE;
	ble_cbuf.size_mask = CSIZE - 1;
}

/***************************************************************************//**
 * @brief
 * 	Pushes string to the circular buffer
 *
 * @details
 * 	This function pushes a string to the circular buffer if there is enough space available
 *
 * @param[in] string
 * 	string to be be added to the circular buffer
 *
 ******************************************************************************/
void ble_circ_push(char *string){
	EFM_ASSERT(ble_circ_space() != 0); // no space (0)
	uint8_t string_len = strlen(string);
	uint8_t packet_size = string_len + 1;

	// check packet size fits in buffer
	if (packet_size <= ble_circ_space()){
		ble_cbuf.cbuf[ble_cbuf.write_ptr] = packet_size;
		update_circ_wrtindex(&ble_cbuf, 1);

		// write each char to buffer
		for (int i = 0; i < string_len; i++){
			ble_cbuf.cbuf[ble_cbuf.write_ptr] = string[i];
			update_circ_wrtindex(&ble_cbuf, 1);
		}
	}

	// if packet too long for available space
	else{
		EFM_ASSERT(false);
	}
}

/***************************************************************************//**
 * @brief
 * 	Check space available in circular buffer
 *
 * @details
 * 	This function returns the amount of space available in the circular buffer
 *
 * @return
 * 	available space in the buffer
 *
 ******************************************************************************/
uint8_t ble_circ_space(void){
	uint8_t space = CSIZE - ((ble_cbuf.write_ptr - ble_cbuf.read_ptr) & ble_cbuf.size_mask);
	return space;
}

/***************************************************************************//**
 * @brief
 * 	Update the write index of the circular buffer
 *
 * @details
 * 	This function will update the read index of the circular buffer by the update_by value. The buffer
 * 	must be sized of a power of 2
 *
 * @param[in] index_struct
 * 	Pointer to the circular buffer
 *
 * @param[in] index_struct
 * 	number to update the write index pointer by
 ******************************************************************************/
void update_circ_wrtindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by){
	index_struct->write_ptr = (index_struct->write_ptr + update_by) & index_struct->size_mask;
}

/***************************************************************************//**
 * @brief
 * 	Update the read index of the circular buffer
 *
 * @details
 * 	This function will update the read index of the circular buffer by the update_by value. The buffer
 * 	must be sized of a power of 2
 *
 * @param[in] index_struct
 * 	Pointer to the circular buffer
 *
 * @param[in] index_struct
 * 	number to update the read index pointer by
 ******************************************************************************/
void update_circ_readindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by){
	index_struct->read_ptr = (index_struct->read_ptr + update_by) & index_struct->size_mask;
}

/***************************************************************************//**
 * @brief
 *   BLE Test performs two functions.  First, it is a Test Driven Development
 *   routine to verify that the LEUART is correctly configured to communicate
 *   with the BLE HM-18 module.  Second, the input argument passed to this
 *   function will be written into the BLE module and become the new name
 *   advertised by the module while it is looking to pair.
 *
 * @details
 * 	 This global function will use polling functions provided by the LEUART
 * 	 driver for both transmit and receive to validate communications with
 * 	 the HM-18 BLE module.  For the assignment, the communication with the
 * 	 BLE module must use low energy design principles of being an interrupt
 * 	 driven state machine.
 *
 * @note
 *   For this test to run to completion, the phone most not be paired with
 *   the BLE module.  In addition for the name to be stored into the module
 *   a breakpoint must be placed at the end of the test routine and stopped
 *   at this breakpoint while in the debugger for a minimum of 5 seconds.
 *
 * @param[in] *mod_name
 *   The name that will be written to the HM-18 BLE module to identify it
 *   while it is advertising over Bluetooth Low Energy.
 *
 * @return
 *   Returns bool true if successfully passed through the tests in this
 *   function.
 ******************************************************************************/

bool ble_test(char *mod_name){
	uint32_t	str_len;

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// This test will limit the test to the proper setup of the LEUART
	// peripheral, routing of the signals to the proper pins, pin
	// configuration, and transmit/reception verification.  The test
	// will communicate with the BLE module using polling routines
	// instead of interrupts.
	// How is polling different than using interrupts?
	// ANSWER: polling is driven by software and requires the MCU to constantly read.
	// 			Interrupts are hardware drivent and allow for the system to rest until needed
	// How does interrupts benefit the system for low energy operation?
	// ANSWER: You can have the MCU in a sleep mode until needed
	// How does interrupts benefit the system that has multiple tasks?
	// ANSWER: It can react to an interrupt instantly
	//			in the process of other tasks.

	// First, you will need to review the DSD HM10 datasheet to determine
	// what the default strings to write data to the BLE module and the
	// expected return statement from the BLE module to test / verify the
	// correct response

	// The test_str is used to tell the BLE module to end a Bluetooth connection
	// such as with your phone.  The ok_str is the result sent from the BLE module
	// to the micro-controller if there was not active BLE connection at the time
	// the break command was sent to the BLE module.
	// Replace the test_str "" with the command to break or end a BLE connection
	// Replace the ok_str "" with the result that will be returned from the BLE
	//   module if there was no BLE connection
	char		test_str[80] = "AT";
	char		ok_str[80] = "OK";


	// output_str will be the string that will program a name to the BLE module.
	// From the DSD HM10 datasheet, what is the command to program a name into
	// the BLE module?
	// The  output_str will be a string concatenation of the DSD HM10 command
	// and the input argument sent to the ble_test() function
	// Replace the output_str "" with the command to change the program name
	// Replace the result_str "" with the first part of the expected result
	//  the backend of the expected response will be concatenated with the
	//  input argument
	char		output_str[80] = "AT+NAME";
	char		result_str[80] = "OK+Set:";


	// To program the name into your module, you must reset the module after you
	// have sent the command to update the modules name.  What is the DSD HM10
	// name to reset the module?
	// Replace the reset_str "" with the command to reset the module
	// Replace the reset_result_str "" with the expected BLE module response to
	//  to the reset command
	char		reset_str[80] = "AT+RESET";
	char		reset_result_str[80] = "OK+RESET";
	char		return_str[80];

	bool		success;
	bool		rx_disabled, rx_en, tx_en;
	uint32_t	status;

	// These are the routines that will build up the entire command and response
	// of programming the name into the BLE module.  Concatenating the command or
	// response with the input argument name
	strcat(output_str, mod_name);
	strcat(result_str, mod_name);

	// The test routine must not alter the function of the configuration of the
	// LEUART driver, but requires certain functionality to insure the logical test
	// of writing and reading to the DSD HM10 module.  The following c-lines of code
	// save the current state of the LEUART driver that will be used later to
	// re-instate the LEUART configuration

	status = leuart_status(HM10_LEUART0);
	if (status & LEUART_STATUS_RXBLOCK) {
		rx_disabled = true;
		// Enabling, unblocking, the receiving of data from the LEUART RX port
		leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXBLOCKDIS);
	}
	else rx_disabled = false;
	if (status & LEUART_STATUS_RXENS) {
		rx_en = true;
	} else {
		rx_en = false;
		// Enabling the receiving of data from the RX port
		leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXEN);
		while (!(leuart_status(HM10_LEUART0) & LEUART_STATUS_RXENS));
	}

	if (status & LEUART_STATUS_TXENS){
		tx_en = true;
	} else {
		// Enabling the transmission of data to the TX port
		leuart_cmd_write(HM10_LEUART0, LEUART_CMD_TXEN);
		while (!(leuart_status(HM10_LEUART0) & LEUART_STATUS_TXENS));
		tx_en = false;
	}
	leuart_cmd_write(HM10_LEUART0, (LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX));

	// This sequence of instructions is sending the break ble connection
	// to the DSD HM10 module.
	// Why is this command required if you want to change the name of the
	// DSD HM10 module?
	// ANSWER:  It needs to be disconnected to be renamed
	str_len = strlen(test_str);
	for (int i = 0; i < str_len; i++){
		leuart_app_transmit_byte(HM10_LEUART0, test_str[i]);
	}

	// What will the ble module response back to this command if there is
	// a current ble connection?
	// ANSWER: OK+LOST
	str_len = strlen(ok_str);
	for (int i = 0; i < str_len; i++){
		return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
		if (ok_str[i] != return_str[i]) {
				EFM_ASSERT(false);;
		}
	}

	// This sequence of code will be writing or programming the name of
	// the module to the DSD HM10
	str_len = strlen(output_str);
	for (int i = 0; i < str_len; i++){
		leuart_app_transmit_byte(HM10_LEUART0, output_str[i]);
	}

	// Here will be the check on the response back from the DSD HM10 on the
	// programming of its name
	str_len = strlen(result_str);
	for (int i = 0; i < str_len; i++){
		return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
		if (result_str[i] != return_str[i]) {
				EFM_ASSERT(false);;
		}
	}

	// It is now time to send the command to RESET the DSD HM10 module
	str_len = strlen(reset_str);
	for (int i = 0; i < str_len; i++){
		leuart_app_transmit_byte(HM10_LEUART0, reset_str[i]);
	}

	// After sending the command to RESET, the DSD HM10 will send a response
	// back to the micro-controller
	str_len = strlen(reset_result_str);
	for (int i = 0; i < str_len; i++){
		return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
		if (reset_result_str[i] != return_str[i]) {
				EFM_ASSERT(false);;
		}
	}

	// After the test and programming have been completed, the original
	// state of the LEUART must be restored
	if (!rx_en) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXDIS);
	if (rx_disabled) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXBLOCKEN);
	if (!tx_en) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_TXDIS);
	leuart_if_reset(HM10_LEUART0);

	success = true;


	CORE_EXIT_CRITICAL();
	return success;
}

/***************************************************************************//**
 * @brief
 *   Circular Buff Test is a Test Driven Development function to validate
 *   that the circular buffer implementation
 *
 * @details
 * 	 This Test Driven Development test has tests integrated into the function
 * 	 to validate that the routines can successfully identify whether there
 * 	 is space available in the circular buffer, the write and index pointers
 * 	 wrap around, and that one or more packets can be pushed and popped from
 * 	 the circular buffer.
 *
 * @note
 *   If anyone of these test will fail, an EFM_ASSERT will occur.  If the
 *   DEBUG_EFM=1 symbol is defined for this project, exiting this function
 *   confirms that the push, pop, and the associated utility functions are
 *   working.
 *
 * @par
 *   There is a test escape that is not possible to test through this
 *   function that will need to be verified by writing several ble_write()s
 *   back to back and verified by checking that these ble_write()s were
 *   successfully transmitted to the phone app.
 *
 ******************************************************************************/

void circular_buff_test(void){
	 bool buff_empty;
	 int test1_len = 50;
	 int test2_len = 25;
	 int test3_len = 5;

	 // Why this 0 initialize of read and write pointer?
	 // Student Response: The buffer needs to be empty at first
	 //
	 ble_cbuf.read_ptr = 0;
	 ble_cbuf.write_ptr = 0;

	 // Why do none of these test strings contain a 0?
	 // Student Response: A 0 will be a null character
	 //
	 for (int i = 0;i < test1_len; i++){
		 test_struct.test_str[0][i] = i+1;
	 }
	 test_struct.test_str[0][test1_len] = 0;

	 for (int i = 0;i < test2_len; i++){
		 test_struct.test_str[1][i] = i + 20;
	 }
	 test_struct.test_str[1][test2_len] = 0;

	 for (int i = 0;i < test3_len; i++){
		 test_struct.test_str[2][i] = i +  35;
	 }
	 test_struct.test_str[2][test3_len] = 0;

	 // What is this test validating?
	 // Student response: This is verifying that the size of our buffer matches the macro we defined in the .h file (64 in our case).

	 EFM_ASSERT(ble_circ_space() == CSIZE);

	 // Why is there only one push to the circular buffer at this stage of the test
	 // Student Response: We will only be testing one push of a single string at this stage
	 //
	 ble_circ_push(&test_struct.test_str[0][0]);

	 // What is this test validating?
	 // Student response: This is checking the size of buffer to verify it equals 64-50-1=13
	 EFM_ASSERT(ble_circ_space() == (CSIZE - test1_len - 1));

	 // Why is the expected buff_empty test = false?
	 // Student Response: Since the buffer will not be empty at this point, it will return false when doing a pop operation
	 //
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == false);
	 for (int i = 0; i < test1_len; i++){
		 EFM_ASSERT(test_struct.test_str[0][i] == test_struct.result_str[i]);
	 }

	 // What is this test validating?
	 // Student response: If the string added to the buffer is the same length as the test string
	 EFM_ASSERT(strlen(test_struct.result_str) == test1_len);

	 // What is this test validating?
	 // Student response: Checking if the buffer is empty
	 EFM_ASSERT(ble_circ_space() == CSIZE);

	 // What does this next push on the circular buffer test?
	 // Student Response: This is testing to see if another string can be added to the buffer

	 ble_circ_push(&test_struct.test_str[1][0]);


	 EFM_ASSERT(ble_circ_space() == (CSIZE - test2_len - 1));

	 // What does this next push on the circular buffer test?
	 // Student Response: This is doing the same as the previous verification but with a different string
	 ble_circ_push(&test_struct.test_str[2][0]);


	 EFM_ASSERT(ble_circ_space() == (CSIZE - test2_len - 1 - test3_len - 1));

	 // What does this next push on the circular buffer test?
	 // Student Response: It's checking to see if the buffer wraps backs around to the 0 index
	 EFM_ASSERT(abs(ble_cbuf.write_ptr - ble_cbuf.read_ptr) < CSIZE);

	 // Why is the expected buff_empty test = false?
	 // Student Response: We still have not fully cleared the buffer and still have 2 strings so it will return false
	 //
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == false);
	 for (int i = 0; i < test2_len; i++){
		 EFM_ASSERT(test_struct.test_str[1][i] == test_struct.result_str[i]);
	 }

	 // What is this test validating?
	 // Student response: If the length of the string in buffer is equal to the second string
	 EFM_ASSERT(strlen(test_struct.result_str) == test2_len);

	 EFM_ASSERT(ble_circ_space() == (CSIZE - test3_len - 1));

	 // Why is the expected buff_empty test = false?
	 // Student Response: We still have to pop one more string in the buffer which returns false
	 //
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == false);
	 for (int i = 0; i < test3_len; i++){
		 EFM_ASSERT(test_struct.test_str[2][i] == test_struct.result_str[i]);
	 }

	 // What is this test validating?
	 // Student response: If the length of the string added to buffer is equal to the third string
	 EFM_ASSERT(strlen(test_struct.result_str) == test3_len);

	 EFM_ASSERT(ble_circ_space() == CSIZE);

	 // Using these three writes and pops to the circular buffer, what other test
	 // could we develop to better test out the circular buffer?
	 // Student Response: We could test other edge cases such as overflowing the buffer, empty strings, etc. Essentially anything else
	 //						that could make the buffer fail so that we can handle those cases.


	 // Why is the expected buff_empty test = true?
	 // Student Response: The buffer is empty at this point so the call will return true
	 //
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == true);
	 ble_write("\nPassed Circular Buffer Test\n");
 }

