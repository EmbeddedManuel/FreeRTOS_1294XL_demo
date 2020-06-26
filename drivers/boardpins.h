/*
 * boardpins.h
 *
 *  Created on: Jun. 10, 2020
 *      Author: m009b
 */

#ifndef DRIVERS_BOARDPINS_H_
#define DRIVERS_BOARDPINS_H_

typedef enum UART_PORT {UART0, UART1, UART2, UART3, UART4, UART5, UART6, UART7, UART_PORTS} uartport_t;

typedef enum GPIO_PORT {
    GPIOA, GPIOB, GPIOC, GPIOD,
    GPIOE, GPIOF, GPIOG, GPIOH,
    GPIOJ, GPIOK, GPIOL, GPIOM,
    GPION, GPIOP, GPIOQ, GPIO_PORTS
} gpioport_t;

void UartPortInit(uartport_t port);

#endif /* DRIVERS_BOARDPINS_H_ */
