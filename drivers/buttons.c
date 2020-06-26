/*
 * buttons.c
 *
 *  Created on: May 27, 2020
 *      Author: m009b
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"

#include "buttons.h"

void buttonsInit(uint8_t buttonConfig, void (*buttonIntHandler)(void)) {
    ASSERT(buttonIntHandler != NULL);

    // Enabling the switch port
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);

    // Setting the Pins that the switches are connected to as inputs
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, buttonConfig);

    // Setting the switches to have minimal strength & to enable the internal weak pull-up
    GPIOPadConfigSet(GPIO_PORTJ_BASE, buttonConfig, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);

    // Initialize the Switch's registered input to 0
    GPIOPinWrite(GPIO_PORTJ_BASE, buttonConfig, 0);

    // Reset the current interrupt configuration & state
    GPIOIntDisable(GPIO_PORTJ_BASE, buttonConfig);
    GPIOIntClear(GPIO_PORTJ_BASE, buttonConfig);

    // Registering the handler function for port J in the vector table & enabling the interrupt
    GPIOIntRegister(GPIO_PORTJ_BASE, buttonIntHandler);

    /*
     * Note:
     *      - In order to simply enabling the interrupt without having to register the handler function
     *          (aka going the "extern handler" in the vector table c file route),
     *        use > IntEnable(INT_GPIOJ_TM4C129);'
     *          (instead of GPIOIntRegister)
     *     - This enables the interrupt on the NVIC side (still needs to be enabled on the GPIO module side)
     */

    // Configuring the interrupt for the port to activate on rising edge, and to only occur on the switch's pins
    GPIOIntTypeSet(GPIO_PORTJ_BASE, buttonConfig, GPIO_FALLING_EDGE);

    // Enable the GPIO module interrupt
    GPIOIntEnable(GPIO_PORTJ_BASE, buttonConfig);
}

uint32_t getButtonStatus(uint8_t buttonConfig) {
    return (~GPIOPinRead(GPIO_PORTJ_BASE, buttonConfig) & buttonConfig);
}

uint32_t resetButtonInterrupt() {
    uint32_t status = GPIOIntStatus(GPIO_PORTJ_BASE, true);
    GPIOIntClear(GPIO_PORTJ_BASE, EITHER_BUTTON);

    return status;
}

void disableButtonInterrupt() {
    GPIOIntDisable(GPIO_PORTJ_BASE, EITHER_BUTTON);
}

void enableButtonInterrupt(uint8_t buttonConfig) {
    // Configuring the interrupt for the port to activate on rising edge, and to only occur on the switch's pins
    GPIOIntTypeSet(GPIO_PORTJ_BASE, buttonConfig, GPIO_FALLING_EDGE);

    // Enable the GPIO module interrupt
    GPIOIntEnable(GPIO_PORTJ_BASE, buttonConfig);
}

