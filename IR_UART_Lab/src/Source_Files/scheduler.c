/**
 * @file scheduler.c
 * @author Ivan Rodriguez
 * @date September 17th, 2020
 * @brief Contains scheduler for events
 *
 */

#include "scheduler.h"

//***********************************************************************************
// Private variables
//***********************************************************************************
static unsigned int event_scheduled;

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * 	Initializes the scheduler
 *
 * @details
 *	Initializes the event_scheduled static variable by setting it to 0
 *
 * @note
 * 	This function will be called when setting up the peripherals
 *
 *
 ******************************************************************************/
void scheduler_open(void){
	event_scheduled = 0; // sets equal to 0
}

/***************************************************************************//**
 * @brief
 * 	Adds event to scheduler
 *
 * @details
 *	This function sets the event_scheduled static variable to the input event
 *
 * @note
 * 	This function will pause interrupt while adding events
 *
 *@param[in] event
 *	event to add to scheduler
 *
 ******************************************************************************/
void add_scheduled_event(uint32_t event){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	event_scheduled |= event;
	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 * 	Removes event from scheduler
 *
 * @details
 *	This function removes the input event
 *
 * @note
 * 	This function will pause interrupts while removing events
 *
 *@param[in] event
 *	event to remove from scheduler
 ******************************************************************************/
void remove_scheduled_event(uint32_t event){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	event_scheduled &= ~event;
	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 * 	Returns currently scheduled events
 *
 * @details
 *	This function returns any events that are set in the scheduler
 *
 *@return
 *	the static event_scheduled variable that includes all scheduled events
 *
 ******************************************************************************/
uint32_t get_scheduled_events(void){
	return event_scheduled;
}
