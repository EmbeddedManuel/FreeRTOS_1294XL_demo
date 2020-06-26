/*
 * buttons.h
 *
 *  Created on: May 27, 2020
 *      Author: m009b
 */

#ifndef DRIVERS_BUTTONS_H_
#define DRIVERS_BUTTONS_H_

#define BUTTON_COUNT    2

#define BTN1            (1 << 0)
#define BTN2            (1 << 1)

#define EITHER_BUTTON   (BTN1 | BTN2)

void buttonsInit(uint8_t buttonConfig, void (*buttonIntHandler)(void));
uint32_t getButtonStatus(uint8_t buttonConfig);
uint32_t resetButtonInterrupt();
void disableButtonInterrupt();
void enableButtonInterrupt(uint8_t buttonConfig);

#endif /* DRIVERS_BUTTONS_H_ */
