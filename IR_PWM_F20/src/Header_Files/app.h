//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef	APP_HG
#define	APP_HG

/* System include statements */


/* Silicon Labs include statements */
#include "em_cmu.h"
#include "em_assert.h"

/* The developer's include statements */
#include "cmu.h"
#include "gpio.h"
#include "letimer.h"
#include "brd_config.h"


//***********************************************************************************
// defined files
//***********************************************************************************
#define		PWM_PER				2.7		// PWM period in seconds
#define		PWM_ACT_PER			0.15	// PWM active period in seconds
#define 	OUT0_EN				true
#define 	OUT1_EN				true
#define		PWM_ROUTE_0			LETIMER_ROUTELOC0_OUT0LOC_LOC28
#define 	PWM_ROUTE_1			LETIMER_ROUTELOC0_OUT1LOC_LOC28



//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void app_peripheral_setup(void);

#endif
