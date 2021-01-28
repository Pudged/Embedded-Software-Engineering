//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef SLEEP_ROUTINES_H
#define SLEEP_ROUTINES_H

/* Silicon Labs include statements */
#include "em_emu.h"
#include "em_int.h"
#include "em_core.h"
#include "em_assert.h"

//***********************************************************************************
// defined files
//***********************************************************************************
#define 	EM0 				0
#define 	EM1 				1
#define 	EM2 				2
#define 	EM3 				3
#define 	EM4 				4
#define 	MAX_ENERGY_MODES 	5

//***********************************************************************************
// function prototypes
//***********************************************************************************
void sleep_open(void);
void sleep_block_mode(uint32_t EM);
void sleep_unblock_mode(uint32_t EM);
void enter_sleep(void);
uint32_t current_block_energy_mode(void);

#endif /* SLEEP_ROUTINES_H */
