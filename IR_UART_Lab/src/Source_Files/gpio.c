/**
 * @file gpio.c
 * @author Ivan Rodriguez
 * @date September 10th, 2020
 * @brief GPIO file
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"


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
 *	Set up GPIO and LEDs for output
 *
 * @details
 *	Enables the CMU clock for GPIO
 *	Sets up GPIO (LED) pins
 *
 * @note
 *	This function is called in app_peripheral_setup
 *
 ******************************************************************************/
void gpio_open(void){

	CMU_ClockEnable(cmuClock_GPIO, true);

	// Configure LED pins
	GPIO_DriveStrengthSet(LED0_PORT, LED0_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED0_PORT, LED0_PIN, LED0_GPIOMODE, LED0_DEFAULT);

	GPIO_DriveStrengthSet(LED1_PORT, LED1_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED1_PORT, LED1_PIN, LED1_GPIOMODE, LED1_DEFAULT);

	// SI7021 pins
	GPIO_DriveStrengthSet(SI7021_SENSOR_EN_PORT, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(SI7021_SENSOR_EN_PORT, SI7021_SENSOR_EN_PIN, gpioModePushPull, true);
	GPIO_PinModeSet(SI7021_SCL_PORT, SI7021_SCL_PIN, gpioModeWiredAnd, true);
	GPIO_PinModeSet(SI7021_SDA_PORT, SI7021_SDA_PIN, gpioModeWiredAnd, true);

	// LEUART0 Pins
	GPIO_DriveStrengthSet(LEUART0_TX_PORT, gpioDriveStrengthStrongAlternateWeak);
	GPIO_PinModeSet(LEUART0_TX_PORT, LEUART0_TX_PIN, gpioModePushPull, true);
	GPIO_PinModeSet(LEUART0_RX_PORT, LEUART0_RX_PIN, gpioModeInput, false);

}
