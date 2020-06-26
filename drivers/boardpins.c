/*
 * boardpins.c
 *
 *  Created on: Jun. 10, 2020
 *      Author: m009b
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"

#include "boardpins.h"

static const uint32_t g_gpioPortPeripheralTable[] =
{
 SYSCTL_PERIPH_GPIOA, SYSCTL_PERIPH_GPIOB, SYSCTL_PERIPH_GPIOC, SYSCTL_PERIPH_GPIOD,
 SYSCTL_PERIPH_GPIOE, SYSCTL_PERIPH_GPIOF, SYSCTL_PERIPH_GPIOG, SYSCTL_PERIPH_GPIOH,
 SYSCTL_PERIPH_GPIOJ, SYSCTL_PERIPH_GPIOK, SYSCTL_PERIPH_GPIOL, SYSCTL_PERIPH_GPIOM,
 SYSCTL_PERIPH_GPION, SYSCTL_PERIPH_GPIOP, SYSCTL_PERIPH_GPIOQ
};

static const uint32_t g_gpioPortBaseTable[] =
{
 GPIO_PORTA_BASE, GPIO_PORTB_BASE, GPIO_PORTC_BASE, GPIO_PORTD_BASE,
 GPIO_PORTE_BASE, GPIO_PORTF_BASE, GPIO_PORTG_BASE, GPIO_PORTH_BASE,
 GPIO_PORTJ_BASE, GPIO_PORTK_BASE, GPIO_PORTL_BASE, GPIO_PORTM_BASE,
 GPIO_PORTN_BASE, GPIO_PORTP_BASE, GPIO_PORTQ_BASE,
};


/*****************************************************************************/
//                                 UART Ports                                //
//                      Configuration tables & functions                     //
/*****************************************************************************/


static const uint32_t g_uartGPIOPeripheralTable[] =
{
 SYSCTL_PERIPH_GPIOA,   // UART 0 peripheral port
 SYSCTL_PERIPH_GPIOB,   // UART 1 peripheral port
 SYSCTL_PERIPH_GPIOD,   // UART 2 peripheral port
 SYSCTL_PERIPH_GPIOJ,   // UART 3 peripheral port
 SYSCTL_PERIPH_GPIOK,   // UART 4 peripheral port
 SYSCTL_PERIPH_GPIOC,   // UART 5 peripheral port
 SYSCTL_PERIPH_GPIOP,   // UART 6 peripheral port
 SYSCTL_PERIPH_GPIOC    // UART 7 peripheral port
};

static const uint32_t g_uartPeripheralTable[] =
{
  SYSCTL_PERIPH_UART0,   // UART 0 peripheral port
  SYSCTL_PERIPH_UART1,   // UART 1 peripheral port
  SYSCTL_PERIPH_UART2,   // UART 2 peripheral port
  SYSCTL_PERIPH_UART3,   // UART 3 peripheral port
  SYSCTL_PERIPH_UART4,   // UART 4 peripheral port
  SYSCTL_PERIPH_UART5,   // UART 5 peripheral port
  SYSCTL_PERIPH_UART6,   // UART 6 peripheral port
  SYSCTL_PERIPH_UART7    // UART 7 peripheral port
};

static const uint32_t g_uartPortBaseTable[] =
{
 GPIO_PORTA_BASE,   // UART 0 port base
 GPIO_PORTB_BASE,   // UART 1 port base
 GPIO_PORTD_BASE,   // UART 2 port base
 GPIO_PORTJ_BASE,   // UART 3 port base
 GPIO_PORTK_BASE,   // UART 4 port base
 GPIO_PORTC_BASE,   // UART 5 port base
 GPIO_PORTH_BASE,   // UART 6 port base
 GPIO_PORTC_BASE    // UART 7 port base
};

static const uint32_t g_uartRxPinMuxTable[] =
{
 GPIO_PA0_U0RX,   // UART 0 TX pin mux mode
 GPIO_PB0_U1RX,   // UART 1 TX pin mux mode
 GPIO_PD4_U2RX,   // UART 2 TX pin mux mode
 GPIO_PJ0_U3RX,   // UART 3 TX pin mux mode
 GPIO_PK0_U4RX,   // UART 4 TX pin mux mode
 GPIO_PC6_U5RX,   // UART 5 TX pin mux mode
 GPIO_PP0_U6RX,   // UART 6 TX pin mux mode
 GPIO_PC4_U7RX    // UART 7 TX pin mux mode
};


static const uint32_t g_uartTxPinMuxTable[] =
{
 GPIO_PA1_U0TX,     // UART 0 RX pin mux mode
 GPIO_PB1_U1TX,     // UART 1 RX pin mux mode
 GPIO_PD5_U2TX,     // UART 2 RX pin mux mode
 GPIO_PJ1_U3TX,     // UART 3 RX pin mux mode
 GPIO_PK1_U4TX,     // UART 4 RX pin mux mode
 GPIO_PC7_U5TX,     // UART 5 RX pin mux mode
 GPIO_PP1_U6TX,     // UART 6 RX pin mux mode
 GPIO_PC5_U7TX      // UART 7 RX pin mux mode
};

static const uint32_t g_uartPinModeTable[] =
{
 (GPIO_PIN_0 | GPIO_PIN_1),     // UART0 pin config: A0 & A1
 (GPIO_PIN_0 | GPIO_PIN_1),     // UART1 pin config: B0 & B1
 (GPIO_PIN_4 | GPIO_PIN_5),     // UART2 pin config: D4 & D5
 (GPIO_PIN_0 | GPIO_PIN_1),     // UART3 pin config: J0 & J1
 (GPIO_PIN_0 | GPIO_PIN_1),     // UART4 pin config: K0 & K1
 (GPIO_PIN_6 | GPIO_PIN_7),     // UART5 pin config: C6 & C7
 (GPIO_PIN_0 | GPIO_PIN_1),     // UART6 pin config: P0 & P1
 (GPIO_PIN_4 | GPIO_PIN_5)      // UART7 pin config: C4 & C5
};


void UartPortInit(uartport_t port) {
    ASSERT(port < UART_PORTS);  // Ensure port number is valid

    SysCtlPeripheralEnable(g_uartGPIOPeripheralTable[port]);
    SysCtlPeripheralEnable(g_uartPeripheralTable[port]);    // Enable Peripheral

    GPIOPinConfigure(g_uartRxPinMuxTable[port]);    // Configure pin mux for the UART's TX pin
    GPIOPinConfigure(g_uartTxPinMuxTable[port]);    // Configure pin mux for the UART's RX pin

    GPIOPinTypeUART(g_uartPortBaseTable[port], g_uartPinModeTable[port]);   // Configure UART's pin mode
}
