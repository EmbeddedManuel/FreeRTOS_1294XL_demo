/*
 * leds.c
 *
 *  Created on: May 20, 2020
 *      Author: Manuel
 */


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "leds.h"

#define LED1_PORT       GPIO_PORTN_BASE
#define LED1_PIN        GPIO_PIN_1

#define LED2_PORT       GPIO_PORTN_BASE
#define LED2_PIN        GPIO_PIN_0

#define LED3_PORT       GPIO_PORTF_BASE
#define LED3_PIN        GPIO_PIN_4

#define LED4_PORT       GPIO_PORTF_BASE
#define LED4_PIN        GPIO_PIN_0

/* Table with the GPIO port register address for a given LED*/
static uint32_t sg_LedPortTable[] =
{
     LED1_PORT, LED2_PORT, LED3_PORT, LED4_PORT
};

/* Table with the pin # for a given LED*/
static uint8_t sg_LedPinTable[] =
{
     LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN
};

/* Table with the bit-mask value for a given LED*/
static uint8_t sg_LedMaskTable[] =
{
     LED1_MASK, LED2_MASK, LED3_MASK, LED4_MASK
};

/*
 * Table with GPIO write value for a given combination of mask + LED write value
 * (mask & value) is used as the index into the table.
 * Table has 9 elements instead of 8 so index 8 can be used (no need for a "-1" to index into table)
 */
static uint8_t sg_LedValueTable[] =
{
    LEDS_OFF,
    LEDS_ON,   LEDS_ON,   LEDS_ON,   LEDS_ON,
    LEDS_ON,   LEDS_ON,   LEDS_ON,   LEDS_ON
};

void ledsInit() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);    // Enabling port N (LED 1 & 2)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);    // Enabling port F (LED 3 & 4)

    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, LED1_PIN | LED2_PIN);    // Setting LED 1 & 2 pins to output
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED3_PIN | LED4_PIN);    // Setting LED 3 & 4 pins to output

    GPIOPadConfigSet(GPIO_PORTN_BASE, LED1_PIN | LED2_PIN,          // Setting output to be limited to 12 mA & push-pull configuration for LEDS 1 & 2.
                         GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);

    GPIOPadConfigSet(GPIO_PORTF_BASE, LED3_PIN | LED4_PIN,          // Setting output to be limited to 12 mA & push-pull configuration for LEDS 3 & 4.
                         GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);

    ledsWrite(ALL_LEDS, LEDS_OFF);  // Initializing all leds to being off.
}

/**
 * @brief   Write Led Values.
 * @param   mask:
 *              bitmask (h0~hF) of all LEDS to write to.
 *              First 4 bits are used, each correspond to an led (bit0 = led1, ...).
 * @param   val:
 *              logical state to toggle the valid LEDs to.
 *              First 4 bits are used, each correspond to an led (bit0 = led1, ...).
 *              0 = off, 1 = on.
 *              A given bit in the value is only considered if the corresponding bit in the mask is set.
 * @details
 *          (Over-complicated way to write LEDs.)
 *          Uses look-up tables to minimize code-size and (hopefully) execution-time.
 *          ExampleS:   Turning on only LED3 (and others off): mask = 0x.f, value = 0x04 (=0b0100)
 *                      Turning off LED1 while not touching any other LED: mask = 0x01, value = 0x00
 *                      Turning on LED1 & LED3: mask = 0x05 (= 0b0101), value = 0xff (2nd and 4th bits are ignored b/c the mask)
 */
void ledsWrite(uint8_t mask, uint8_t val) {
    int i = 0;

    // Go through the bitmask bit by bit
    for(; i < LED_COUNT; i++) {
        if (mask & sg_LedMaskTable[i]) {
            GPIOPinWrite(sg_LedPortTable[i], sg_LedPinTable[i], sg_LedValueTable[val & sg_LedMaskTable[i]]);
        }
    }
}

/**
 * @brief   Sets value to a specific LED.
 * @param   led:
 *              LED to write value to.
 *              Value should be between 1~4 to correspond how the LEDs are labeled on the board (D1~D4).
 * @param   val:
 *              Value to write to led.
 *              Value to be either 0 (OFF) or 1 (ON).
 */
void ledSet(uint8_t led, uint8_t val) {
    if (led == 0) {
        return; // Just so things don't get weird.
    }

    led = (led-1 & (LED_COUNT-1));  // Makes sure led is asserted between 0~3

    GPIOPinWrite(sg_LedPortTable[led], sg_LedPinTable[led], sg_LedValueTable[(val & 1)]);
}

uint8_t ledsRead() {
    uint8_t retval = 0;

    int i = 0;
    for(; i < LED_COUNT; i++) {
        if (GPIOPinRead(sg_LedPortTable[i], sg_LedPinTable[i])) {
            retval |= 1 << i;
        }
    }

    return retval;
}
