/*
 * frtosUARTstdio.h
 *
 *  Created on: Jun. 10, 2020
 *      Author: m009b
 */

#ifndef FREERTOS_UTILS_FRTOSUARTSTDIO_H_
#define FREERTOS_UTILS_FRTOSUARTSTDIO_H_

#define STDIO_UART_PORT     0

void vUartStdioInit(uint32_t baudRate, uint32_t sysClkRate);
inline int vUARTputs(const char *buf);
int vUARTgets(char *buf, uint32_t len);
void vUARTprintf(const char *pcString, ...);

#endif /* FREERTOS_UTILS_FRTOSUARTSTDIO_H_ */
