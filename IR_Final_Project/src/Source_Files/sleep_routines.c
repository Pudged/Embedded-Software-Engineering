/**
 * @file sleep_routines.c
 * @brief EM Manager for Application
 ***************************************************************************
 * @section License
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
**/

#include "sleep_routines.h"

//***********************************************************************************
// Static / Private Variables
//***********************************************************************************
static int lowest_energy_mode [MAX_ENERGY_MODES];

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * 	Initializes the Energy Mode manager
 *
 * @details
 *	Unblocks every energy mode (EM0-EM4)

 ******************************************************************************/
void sleep_open(){
	for (int i = 0; i < MAX_ENERGY_MODES; i++){
		lowest_energy_mode[i] = 0;
	}
}

/***************************************************************************//**
 * @brief
 * 	Block EM
 *
 * @details
 *	This function blocks the input energy mode from being used
 *
 * @note
 * 	This function will pause interrupts
 *
 *@param[in] EM
 *	Energy Mode to be blocked
 ******************************************************************************/
void sleep_block_mode(uint32_t EM){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	lowest_energy_mode[EM] = 1;
	EFM_ASSERT(lowest_energy_mode[EM] < 5);

	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 * 	Unblock EM
 *
 * @details
 *	This function unblocks the input energy mode so that it can be used
 *
 * @note
 * 	This function will pause interrupts
 *
 *@param[in] EM
 *	Energy Mode to be unblocked
 ******************************************************************************/
void sleep_unblock_mode(uint32_t EM){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	lowest_energy_mode[EM]= 0;
	EFM_ASSERT(lowest_energy_mode[EM] >= 0);

	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 * 	Enters Sleep Mode
 *
 * @details
 *	This function places the processor into sleep mode

 ******************************************************************************/
void enter_sleep(void){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	if (lowest_energy_mode[EM0] > 0){}
	else if (lowest_energy_mode[EM1] > 0){}
	else if (lowest_energy_mode[EM2] > 0)
		EMU_EnterEM1();
	else if (lowest_energy_mode[EM3] > 0)
		EMU_EnterEM2(true);
	else EMU_EnterEM3(true);
	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 * 	Returns the current energy mode
 *
 * @details
 *	This function accesses the static lowest_energy_mode variable and returns it
 *	when called
 *
 *@return
 *	The current lowest energy mode enabled
 *
 ******************************************************************************/
uint32_t current_block_energy_mode(void){
	for (int i = 0; i < MAX_ENERGY_MODES; i++){
		if (lowest_energy_mode[i] != 0)
			return i;
	}
	return MAX_ENERGY_MODES - 1;
}

