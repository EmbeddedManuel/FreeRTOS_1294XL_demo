/*
 * leds.h
 *
 *  Created on: May 20, 2020
 *      Author: Manuel
 */

#ifndef DRIVERS_NOKERNEL_LEDS_H_
#define DRIVERS_NOKERNEL_LEDS_H_

#define LED_COUNT   4

#define LED1_MASK   1
#define LED2_MASK   2
#define LED3_MASK   4
#define LED4_MASK   8

#define ALL_LEDS    0xFF

#define LEDS_ON     0xFF
#define LEDS_OFF    0x00

#define LED1        1
#define LED2        2
#define LED3        3
#define LED4        4

#define LED_ON      1
#define LED_OFF     0


void ledsInit();
void ledsWrite(uint8_t mask, uint8_t val);
void ledSet(uint8_t led, uint8_t val);
uint8_t ledsRead();


#endif /* DRIVERS_NOKERNEL_LEDS_H_ */
