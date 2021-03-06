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


//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Static / Private Variables
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
	letimer_start(LETIMER0, true);

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

	uint32_t currentEM = current_block_energy_mode();
	sleep_unblock_mode(currentEM);

	if (currentEM < EM4)
		sleep_block_mode(currentEM + 1);
	else
		sleep_block_mode(EM0);
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

