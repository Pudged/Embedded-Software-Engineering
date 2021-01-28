/**
 * @file cmu.c
 * @author Ivan Rodriguez
 * @date September 10th, 2020
 * @brief Clock Management Unit file
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "cmu.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************


//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Set up the CMU
 *
 * @details
 *	Sets up the Clock Management Unit and routes the LETIMER through the clock tree
 *
 * @note
 *	This function is called in app_peripheral_setup
 *
 ******************************************************************************/
void cmu_open(void){

		CMU_ClockEnable(cmuClock_HFPER, true);

		// By default, Low Frequency Resistor Capacitor Oscillator, LFRCO, is enabled,
		// Disable the LFRCO oscillator
		CMU_OscillatorEnable(cmuOsc_LFRCO  , false, false);	 // What is the enumeration required for LFRCO?

		// Enable the Low Frequency Crystal Oscillator, LFXO (UART)
		CMU_OscillatorEnable(cmuOsc_LFXO , true, true);	// What is the enumeration required for LFXO?
		CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO); // route for LEUART0 on clock tree

		// No requirement to enable the ULFRCO oscillator.  It is always enabled in EM0-4H

		// Route LF clock to LETIMER0 clock tree
		CMU_ClockSelectSet(cmuClock_LFA , cmuSelect_ULFRCO);	// What clock tree does the LETIMER0 reside on?

		// Now, you must ensure that the global Low Frequency is enabled
		CMU_ClockEnable(cmuClock_CORELE, true);	//This enumeration is found in the Lab 2 assignment

}

