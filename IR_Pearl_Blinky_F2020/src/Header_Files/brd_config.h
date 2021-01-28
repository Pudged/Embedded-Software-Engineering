#ifndef BRD_CONFIG_HG
#define BRD_CONFIG_HG

//***********************************************************************************
// Include files
//***********************************************************************************
#include "em_gpio.h"

//***********************************************************************************
// defined files
//***********************************************************************************

// GPIO pin setup
//#define STRONG_DRIVE

// LED 0 pin is 								// PF4
#define	LED0_PORT				gpioPortF		// Port F
#define LED0_PIN				4u				// Pin 4
#define LED0_DEFAULT			false 			// Default false (0) = off, true (1) = on
#define LED0_GPIOMODE			gpioModePushPull

// LED 1 pin is 								// PF5
#define LED1_PORT				gpioPortF		// Port F
#define LED1_PIN				5u				// Pin 5
#define LED1_DEFAULT			false	// Default false (0) = off, true (1) = on
#define LED1_GPIOMODE			gpioModePushPull

#ifdef STRONG_DRIVE
	#define LED0_DRIVE_STRENGTH		gpioDriveStrengthStrongAlternateStrong
	#define LED1_DRIVE_STRENGTH		gpioDriveStrengthStrongAlternateStrong
#else
	#define LED0_DRIVE_STRENGTH		gpioDriveStrengthWeakAlternateWeak
	#define LED1_DRIVE_STRENGTH		gpioDriveStrengthWeakAlternateWeak
#endif


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************

#endif
