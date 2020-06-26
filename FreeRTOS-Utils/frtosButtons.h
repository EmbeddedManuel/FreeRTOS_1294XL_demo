/*
 * switches.h
 *
 *  Created on: May 23, 2020
 *      Author: m009b
 */

#ifndef FREERTOS_BUTTONS_H_
#define FREERTOS_BUTTONS_H_

#include "FreeRTOS.h"
#include "FreeRTOS/include/event_groups.h"
#include "buttons.h"

#define BUTTON_CONFIG   EITHER_BUTTON

#define DEBOUNCE_DELAY_MS  (50/portTICK_PERIOD_MS) // 100ms

void vButtonsInit();
EventBits_t xGetButtonEvent(uint8_t SwitchMask, TickType_t waitTicks);
EventBits_t xWaitForButtonEvent(EventBits_t SwitchMask);

#endif /* FREERTOS_BUTTONS_H_ */
