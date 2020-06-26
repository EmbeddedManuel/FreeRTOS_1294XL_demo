/*
 * sysinit.c
 *
 *  Created on: May 23, 2020
 *      Author: m009b
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/debug.h"

#include "leds.h"

#include "FreeRTOS.h"
#include "frtosButtons.h"

#include "sysinit.h"

void sysinit() {
    uint32_t sysclk = SysCtlClockFreqSet(
            (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
            SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
           SYSTEM_CLOCK);

    ASSERT(sysclk == SYSTEM_CLOCK);

    ledsInit();
    vButtonsInit();
}
