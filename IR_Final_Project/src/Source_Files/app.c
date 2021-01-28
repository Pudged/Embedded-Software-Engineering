/**
 * @file app.c
 * @author Ivan Rodriguez
 * @date September 10th, 2020
 * @brief Application file
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "app.h"
#include "stdio.h"

//***********************************************************************************
// defined files
//***********************************************************************************
//#define BLE_TEST_ENABLED
#define SI7021_TEST

//***********************************************************************************
// Static / Private Variables
//***********************************************************************************
static char output_str[64];

//***********************************************************************************
// Global Variables
//***********************************************************************************


//***********************************************************************************
// Private functions
//***********************************************************************************

static void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route);

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Set up peripherals used
 *
 * @details
 *	Sets up CMU, GPIO, and LETIMER peripherals
 *
 * @note
 *	This function is called only once at the beginning to setup the peripherals.
 *	It calls several other functions to open up certain peripherals
 *
 ******************************************************************************/

void app_peripheral_setup(void){
	scheduler_open();
	sleep_open();
	cmu_open();
	gpio_open();
	app_letimer_pwm_open(PWM_PER, PWM_ACT_PER, PWM_ROUTE_0, PWM_ROUTE_1);
	//letimer_start(LETIMER0, true); // moved to BOOT UP callback function
	si7021_i2c_open();
	sleep_block_mode(SYSTEM_BLOCK_EM);
	ble_open(BLE_TX_DONE_CB, BLE_RX_DONE_CB);
	add_scheduled_event(BOOT_UP_CB);

}

/***************************************************************************//**
 * @brief
 *	Sets up a PWM signal on the LETIMER
 *
 * @details
 *	This function sets up a struct of APP_LETIMER_PWM_TypeDef
 *	that is then used to setup up the specific LETIMER to be used.
 *
 * @note
 *	This function is called in the peripheral setup function
 *
 * @param[in] period
 *	PWM period time
 *
 * @param[in] act_period
 * 	PWM active period time
 *
 * @param[in] out0_route
 * 	The routing location for PF5
 *
 * @param[in] out1_route
 *	The routing location for PF4
 *
 ******************************************************************************/
void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route){
	// Initializing LETIMER0 for PWM operation by creating the
	// letimer_pwm_struct and initializing all of its elements

	APP_LETIMER_PWM_TypeDef letimer_pwm_struct;

	letimer_pwm_struct.debugRun = false;
	letimer_pwm_struct.enable = false;
	letimer_pwm_struct.out_pin_route0 = out0_route;
	letimer_pwm_struct.out_pin_route1 = out1_route;
	letimer_pwm_struct.out_pin_0_en = OUT0_EN;
	letimer_pwm_struct.out_pin_1_en = OUT1_EN;
	letimer_pwm_struct.period = period;
	letimer_pwm_struct.active_period = act_period;
	letimer_pwm_struct.uf_irq_enable = true;
	letimer_pwm_struct.comp0_irq_enable = false;
	letimer_pwm_struct.comp1_irq_enable = false;
	letimer_pwm_struct.comp0_cb = LETIMER0_COMP0_CB;
	letimer_pwm_struct.comp1_cb = LETIMER0_COMP1_CB;
	letimer_pwm_struct.uf_cb = LETIMER0_UF_CB;

	letimer_pwm_open(LETIMER0, &letimer_pwm_struct);

	// letimer_start will inform the LETIMER0 peripheral to begin counting.
	letimer_start(LETIMER0, true);
}

/***************************************************************************//**
 * @brief
 * 	Callback for UF events in the LETIMER0 peripheral
 *
 * @details
 *	This function handles a UF event by clearing it and properly setting the energy mode

 ******************************************************************************/
void scheduled_letimer0_uf_cb(void){
	EFM_ASSERT(get_scheduled_events() & LETIMER0_UF_CB);
	remove_scheduled_event(LETIMER0_UF_CB);

	si7021_read(SI7021_READ_TEMP_CB, TWO_BYTES, SI7021_READ_TEMP_CMD);
	while(i2c_busy()); // wait until done w/ previous op
	si7021_read(SI7021_READ_RH_CB, TWO_BYTES, SI7021_READ_RH_CMD);
}

/***************************************************************************//**
 * @brief
 * 	Callback for COMP0 events in the LETIMER0 peripheral
 *
 * @details
 *	This function handles a COMP0 event by clearing it and properly setting the energy mode

 ******************************************************************************/
void scheduled_letimer0_comp0_cb(void){
	EFM_ASSERT(get_scheduled_events() & LETIMER0_COMP0_CB);
	remove_scheduled_event(LETIMER0_COMP0_CB);
	EFM_ASSERT(false);
}

/***************************************************************************//**
 * @brief
 * 	Callback for COMP1 events in the LETIMER0 peripheral
 *
 * @details
 *	This function handles a COMP1 event by clearing it and properly setting the energy mode
 *
 ******************************************************************************/
void scheduled_letimer0_comp1_cb(void){
	EFM_ASSERT(get_scheduled_events() & LETIMER0_COMP1_CB);
	remove_scheduled_event(LETIMER0_COMP1_CB);
	EFM_ASSERT(false);
}

/***************************************************************************//**
 * @brief
 *	Callback for SI7021 Temperature Read events from i2c communication
 *
 * @details
 *	This function handles the SI7021 Temperature Read event and toggles LED1 based on the temperature reading from the
 *	SI7021 sensor. On if > 80 F, off if else
 *
 ******************************************************************************/
void scheduled_si7021_read_temp_cb(void){
	EFM_ASSERT(get_scheduled_events() & SI7021_READ_TEMP_CB);
	remove_scheduled_event(SI7021_READ_TEMP_CB);

	float temperatureF = si7021_temperatureF();

	if (temperatureF > 80){
		GPIO_PinOutSet(LED1_PORT, LED1_PIN);
	}
	else{
		GPIO_PinOutClear(LED1_PORT, LED1_PIN);
	}

	sprintf(output_str, "Temp = %d.%d F\n", (int)temperatureF, (int)(temperatureF * 10) % 10);
	ble_write(output_str);
}

/***************************************************************************//**
 * @brief
 *	Callback for SI7021 read RH events from i2c communication
 *
 * @details
 *	This function handles the SI7021 Read Relative Humidity Event
 *
 ******************************************************************************/
void scheduled_si7021_read_rh_cb(void){
	EFM_ASSERT(get_scheduled_events() & SI7021_READ_RH_CB);
	remove_scheduled_event(SI7021_READ_RH_CB);

	float rh = si7021_rh();
	sprintf(output_str, "RH = %d.%d Percent\n", (int)rh, (int)(rh * 10) % 10);
	ble_write(output_str);

}

/***************************************************************************//**
 * @brief
 *	Callback for boot up event
 *
 * @details
 *	This function will run once on boot up (when app_peripheral_setup is called) and begin the letimer0 for PWM.
 *	Then it will transmit the Hello World message to the DSD-Tech HM-10 BLE module.
 *
 * @note
 * 	Uses the BLE_TEST_ENABLED header guard to determine to run the TDD ble_test function written by Professor Graham.
 *
 ******************************************************************************/
void scheduled_boot_up_cb(void){
	EFM_ASSERT(get_scheduled_events() & BOOT_UP_CB);
	remove_scheduled_event(BOOT_UP_CB);

	// ble test
	#ifdef BLE_TEST_ENABLED
		bool success = ble_test("IvanBLE");
		timer_delay(2000);
		EFM_ASSERT(success);
	#endif

	#ifdef CIRC_TEST
		circular_buff_test();
	#endif

	#ifdef SI7021_TEST
		bool success2 = si7021_test();
		EFM_ASSERT(success2);
		ble_write("Passed SI7021 I2C TDD Test\n");
	#endif

	ble_write("\nHello World\n");
	ble_write("Final Project\n");
	ble_write("Ivan\n");

	letimer_start(LETIMER0, true);
}

/***************************************************************************//**
 * @brief
 *	Callback for BLE transmission done events
 *
 * @details
 *	This function handles the BLE transmission complete event once the UART peripheral has completed
 *	transmitting and pops the string.
 *
 ******************************************************************************/
void scheduled_ble_tx_done_cb(){
	EFM_ASSERT(get_scheduled_events() & BLE_TX_DONE_CB);
	remove_scheduled_event(BLE_TX_DONE_CB);

	ble_circ_pop(false);
}

