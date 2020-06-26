/*
 * frtosUARTstdio.c
 *
 *  Created on: Jun. 10, 2020
 *      Author: m009b
 */


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"

#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "boardpins.h"

#include "FreeRTOS-Utils/frtosUARTstdio.h"

typedef struct UARTSTDIO {
    SemaphoreHandle_t   mutex;
    TaskHandle_t        waitTask;
    uint32_t            baseAddr;
    uint32_t            intAddr;
    char*               bufPtr;
    uint32_t            bufLen;
    uint32_t            bufMax;
} uartstdio_t;

static const uint32_t g_UARTBaseTable[] =
{
    UART0_BASE, UART1_BASE, UART2_BASE,
    UART4_BASE, UART5_BASE, UART6_BASE, UART7_BASE
};

static const uint32_t g_UARTIntTable[] =
{
    INT_UART0, INT_UART1, INT_UART2, INT_UART3,
    INT_UART4, INT_UART5, INT_UART6, INT_UART7
};

#define UART_CONFIG     (UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE | UART_CONFIG_WLEN_8)

static const char *const g_pcHex = "0123456789abcdef";

static uartstdio_t uart;

void UartStdioHandler();

/**
 * @brief   Initializes the UART port and FreeRTOS control structures for accessing UART in stdio mode.
 *          UART stdio mode uses standard UART configuration: 8 bit data words, one stop bit, no parity.
 * @param   baudRate: Baud rate to run the UART on.
 *                  Common Baud rates: 9600, 57600, 115200.
 * @param   sysClkRate: Clock frequency that the CPU runs on.
 * @details Currently the driver only allows for one UART port to be configured in stdio mode.
 *          Doing so prevents having to configure the driver and to make decisions on memory management/allocation.
 *          It also simplifies the logic in the driver (specially when handling interrupts).
 */
void vUartStdioInit(uint32_t baudRate, uint32_t sysClkRate) {
    uart.baseAddr = g_UARTBaseTable[STDIO_UART_PORT];
    uart.intAddr = g_UARTIntTable[STDIO_UART_PORT];

    uart.mutex = xSemaphoreCreateMutex();
    uart.waitTask = NULL;

    ASSERT(uart.mutex != NULL);

    uart.bufPtr = NULL;
    uart.bufLen = 0;
    uart.bufMax = 0;

    UartPortInit((uartport_t)STDIO_UART_PORT);
    UARTConfigSetExpClk(uart.baseAddr, sysClkRate, baudRate, UART_CONFIG);

    UARTFIFOLevelSet(uart.baseAddr, UART_FIFO_TX1_8, UART_FIFO_RX1_8);

    UARTIntDisable(uart.baseAddr, 0xFFFFFFFF);

    UARTIntRegister(uart.baseAddr, UartStdioHandler);

    UARTEnable(uart.baseAddr);
}

/**
 * @brief   Fills the transmit FIFO and data register with data
 *          from the buffer pointer in the uartstdio data structure.
 * @return  number of bytes tranmitted.
 * @todo    transfer this to hal_uart when that gets created.
 */
int pvUARTtransmit() {
    taskENTER_CRITICAL();

    int bytes = 0;

    while (UARTSpaceAvail(uart.baseAddr) && uart.bufLen > 0) {
        UARTCharPutNonBlocking(uart.baseAddr, *uart.bufPtr);
        uart.bufLen--;
        uart.bufPtr++;

        bytes++;
    }

    taskEXIT_CRITICAL();

    return bytes;
}

/**
 * @brief   Initializes Initializes the stdio with the transmit buffer and length,
 *          starts transmission and enables interrupt if buffer wasn't fully transmitted.
 * @param   buf: pointer to character buffer to transmit
 * @param   len: length of character buffer.
 * @return  bytes transmitted (which will be len).
 */
int pvUARTwrite(const char *buf, uint32_t len) {
    uart.bufPtr = (char *)buf;
    uart.bufLen = len;
    uart.bufMax = len;

    pvUARTtransmit();

    if (uart.bufLen > 0) {
        UARTIntEnable(uart.baseAddr, UART_INT_TX);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }

    return uart.bufMax;
}

/**
 * @brief   Sends null-terminated string to UART.
 * @param   buf: pointer to null-terminated string.
 * @return  Bytes transmitted.
 */
inline int vUARTputs(const char *buf) {
    ASSERT(buf != NULL);

    TaskHandle_t taskHandle = xTaskGetCurrentTaskHandle();

    xSemaphoreTake(uart.mutex, portMAX_DELAY);

    uart.waitTask = taskHandle;

    int retval = pvUARTwrite(buf, sizeof(buf));

    xSemaphoreGive(uart.mutex);

    return retval;
}

/**
 * @brief   Get null-terminated string from UART.
 * @param   buf: pointer to character buffer to fill with characters.
 * @param   len: max length that the character buffer supports.
 * @return  number of bytes written to the buffer (includes null-terminator).
 * @details Once this function is called,
 *          UART driver will start buffering characters
 *          until a "new-line" is received.
 */
int vUARTgets(char *buf, uint32_t len) {
    if (buf == NULL || len == 0)    return 0;

    TaskHandle_t taskHandle = xTaskGetCurrentTaskHandle();

    xSemaphoreTake(uart.mutex, portMAX_DELAY);

    uart.bufPtr = buf;
    uart.bufLen = 0;
    uart.bufMax = len;
    uart.waitTask = taskHandle;

    UARTIntEnable(uart.baseAddr, (UART_INT_RX | UART_INT_RT));

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    xSemaphoreGive(uart.mutex);

    return uart.bufLen;
}

/**
 * @brief   A vprintf-like function that supports \%c, \%d, \%p, \%s, \%u,
 *          \%x, and \%X.
 * @param   pcString: String to format
 * @param   vaArgP: The variable argument list pointer whose content will
 *                  depend upon the format string passed in pcString.
 * @details Function behaves similar to vprintf from the C library.
 *          The general behaviour of this function is that the string and the
 *          argument will be parsed through and outputted to UART when ready.
 *          So something like "Hello %d World" - Function will first output
 *          "Hello " to uart, then figure out the number & output it,
 *          then output " World".
 * @details Formats supported:
 *          \%c to print a character
 *          \%d or \%i to print a decimal value
 *          \%s to print a string
 *          \%u to print an unsigned decimal value
 *          \%x or \%X to print a hexadecimal value using lower case letters.
 *          \%p to print a pointer as a hexadecimal value
 *          \%\% to print out a \% character
 * @details Function supports number filling formatting,
 *          such as %8d (to represent number with 8 digits max)
 *          or %08d (to represent number with 8 digits with trailing zeros if needed).
 * @details Function was sourced from uartstdio.c provided in TivaWare utilities.
 *          I made some slight changes to how it outputs and re-did the comment
 *          head here to fit into my style.
 */
void pvUARTvprintf(const char *pcString, va_list vaArgP)
{
    uint32_t ui32Idx, ui32Value, ui32Pos, ui32Count, ui32Base, ui32Neg;
    char *pcStr, pcBuf[16], cFill;

    //
    // Check the arguments.
    //
    ASSERT(pcString != 0);

    //
    // Loop while there are more characters in the string.
    //
    while (*pcString)
    {
        //
        // Find the first non-% character, or the end of the string.
        //
        for (ui32Idx = 0;
                (pcString[ui32Idx] != '%') && (pcString[ui32Idx] != '\0');
                ui32Idx++)
        {
        }

        //
        // Write this portion of the string.
        //
        pvUARTwrite(pcString, ui32Idx);

        //
        // Skip the portion of the string that was written.
        //
        pcString += ui32Idx;

        //
        // See if the next character is a %.
        //
        if (*pcString == '%')
        {
            //
            // Skip the %.
            //
            pcString++;

            //
            // Set the digit count to zero, and the fill character to space
            // (in other words, to the defaults).
            //
            ui32Count = 0;
            cFill = ' ';

            //
            // It may be necessary to get back here to process more characters.
            // Goto's aren't pretty, but effective.  I feel extremely dirty for
            // using not one but two of the beasts.
            //
again:

            //
            // Determine how to handle the next character.
            //
            switch (*pcString++)
            {
                //
                // Handle the digit characters.
                //
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                {
                    //
                    // If this is a zero, and it is the first digit, then the
                    // fill character is a zero instead of a space.
                    //
                    if ((pcString[-1] == '0') && (ui32Count == 0))
                    {
                        cFill = '0';
                    }

                    //
                    // Update the digit count.
                    //
                    ui32Count *= 10;
                    ui32Count += pcString[-1] - '0';

                    //
                    // Get the next character.
                    //
                    goto again;
                }

                //
                // Handle the %c command.
                //
                case 'c':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ui32Value = va_arg(vaArgP, uint32_t);

                    //
                    // Print out the character.
                    //
                    pvUARTwrite((char *)&ui32Value, 1);

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle the %d and %i commands.
                //
                case 'd':
                case 'i':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ui32Value = va_arg(vaArgP, uint32_t);

                    //
                    // Reset the buffer position.
                    //
                    ui32Pos = 0;

                    //
                    // If the value is negative, make it positive and indicate
                    // that a minus sign is needed.
                    //
                    if ((int32_t)ui32Value < 0)
                    {
                        //
                        // Make the value positive.
                        //
                        ui32Value = -(int32_t)ui32Value;

                        //
                        // Indicate that the value is negative.
                        //
                        ui32Neg = 1;
                    }
                    else
                    {
                        //
                        // Indicate that the value is positive so that a minus
                        // sign isn't inserted.
                        //
                        ui32Neg = 0;
                    }

                    //
                    // Set the base to 10.
                    //
                    ui32Base = 10;

                    //
                    // Convert the value to ASCII.
                    //
                    goto convert;
                }

                //
                // Handle the %s command.
                //
                case 's':
                {
                    //
                    // Get the string pointer from the varargs.
                    //
                    pcStr = va_arg(vaArgP, char *);

                    //
                    // Determine the length of the string.
                    //
                    for (ui32Idx = 0; pcStr[ui32Idx] != '\0'; ui32Idx++)
                    {
                    }

                    //
                    // Write the string.
                    //
                    pvUARTwrite(pcStr, ui32Idx);

                    //
                    // Write any required padding spaces
                    //
                    if (ui32Count > ui32Idx)
                    {
                        ui32Count -= ui32Idx;

                        while (ui32Count--)
                        {
                            pvUARTwrite(" ", 1);
                        }
                    }

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle the %u command.
                //
                case 'u':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ui32Value = va_arg(vaArgP, uint32_t);

                    //
                    // Reset the buffer position.
                    //
                    ui32Pos = 0;

                    //
                    // Set the base to 10.
                    //
                    ui32Base = 10;

                    //
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    //
                    ui32Neg = 0;

                    //
                    // Convert the value to ASCII.
                    //
                    goto convert;
                }

                //
                // Handle the %x and %X commands.  Note that they are treated
                // identically; in other words, %X will use lower case letters
                // for a-f instead of the upper case letters it should use.  We
                // also alias %p to %x.
                //
                case 'x':
                case 'X':
                case 'p':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ui32Value = va_arg(vaArgP, uint32_t);

                    //
                    // Reset the buffer position.
                    //
                    ui32Pos = 0;

                    //
                    // Set the base to 16.
                    //
                    ui32Base = 16;

                    //
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    //
                    ui32Neg = 0;

                    //
                    // Determine the number of digits in the string version of
                    // the value.
                    //
convert:

                    for (ui32Idx = 1;
                            (((ui32Idx * ui32Base) <= ui32Value) &&
                             (((ui32Idx * ui32Base) / ui32Base) == ui32Idx));
                            ui32Idx *= ui32Base, ui32Count--)
                    {
                    }

                    //
                    // If the value is negative, reduce the count of padding
                    // characters needed.
                    //
                    if (ui32Neg)
                    {
                        ui32Count--;
                    }

                    //
                    // If the value is negative and the value is padded with
                    // zeros, then place the minus sign before the padding.
                    //
                    if (ui32Neg && (cFill == '0'))
                    {
                        //
                        // Place the minus sign in the output buffer.
                        //
                        pcBuf[ui32Pos++] = '-';

                        //
                        // The minus sign has been placed, so turn off the
                        // negative flag.
                        //
                        ui32Neg = 0;
                    }

                    //
                    // Provide additional padding at the beginning of the
                    // string conversion if needed.
                    //
                    if ((ui32Count > 1) && (ui32Count < 16))
                    {
                        for (ui32Count--; ui32Count; ui32Count--)
                        {
                            pcBuf[ui32Pos++] = cFill;
                        }
                    }

                    //
                    // If the value is negative, then place the minus sign
                    // before the number.
                    //
                    if (ui32Neg)
                    {
                        //
                        // Place the minus sign in the output buffer.
                        //
                        pcBuf[ui32Pos++] = '-';
                    }

                    //
                    // Convert the value into a string.
                    //
                    for (; ui32Idx; ui32Idx /= ui32Base)
                    {
                        pcBuf[ui32Pos++] =
                            g_pcHex[(ui32Value / ui32Idx) % ui32Base];
                    }

                    //
                    // Write the string.
                    //
                    pvUARTwrite(pcBuf, ui32Pos);

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle the %% command.
                //
                case '%':
                {
                    //
                    // Simply write a single %.
                    //
                    pvUARTwrite(pcString - 1, 1);

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle all other commands.
                //
                default:
                {
                    //
                    // Indicate an error.
                    //
                    pvUARTwrite("ERROR", 5);

                    //
                    // This command has been handled.
                    //
                    break;
                }
            }
        }
    }
}

/**
 * @brief   A printf-like function for UART that supports \%c, \%d, \%p, \%s, \%u,
 *          \%x, and \%X.
 * @param   pcString: String to format
 * @param   ...: are the optional arguments, which depend on the contents of the
 *               format string.
 * @details Function behaves similar to printf from the C library.
 *          It will parse through the string and arguments and output them to UART.
 * @details Formats supported:
 *          \%c to print a character
 *          \%d or \%i to print a decimal value
 *          \%s to print a string
 *          \%u to print an unsigned decimal value
 *          \%x or \%X to print a hexadecimal value using lower case letters.
 *          \%p to print a pointer as a hexadecimal value
 *          \%\% to print out a \% character
 * @details Function supports number filling formatting,
 *          such as %8d (to represent number with 8 digits max)
 *          or %08d (to represent number with 8 digits with trailing zeros if needed).
 * @details Function was sourced from uartstdio.c provided in TivaWare utilities.
 *          I made some slight changes to how it outputs and re-did the comment
 *          head here to fit into my style.
 */
void
vUARTprintf(const char *pcString, ...)
{
    va_list vaArgP;

    //
    // Start the varargs processing.
    //
    va_start(vaArgP, pcString);

    TaskHandle_t taskHandle = xTaskGetCurrentTaskHandle();

    xSemaphoreTake(uart.mutex, portMAX_DELAY);

    uart.waitTask = taskHandle;

    pvUARTvprintf(pcString, vaArgP);

    xSemaphoreGive(uart.mutex);

    //
    // We're finished with the varargs now.
    //
    va_end(vaArgP);
}

/**
 * @brief   UART stdio interrupt handler.
 * @details It's only active when a task is using UART stdio.
 *          It handles recalling the uart transmit, small receive control (backspace handling),
 *          placing characters in the buffer and waking up tasks when the procedure is complete.
 */
void UartStdioHandler() {

    uint32_t intStatus = UARTIntStatus(uart.baseAddr, true);
    UARTIntClear(uart.baseAddr, intStatus);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    char rxChar;

    if (intStatus & UART_INT_TX) {
        pvUARTtransmit();

        if (uart.bufLen == 0) {
            UARTIntDisable(uart.baseAddr, UART_INT_TX);
            vTaskNotifyGiveFromISR(uart.waitTask, &xHigherPriorityTaskWoken);
        }
    }

    if (intStatus & (UART_INT_RX | UART_INT_RT)) {
        while (UARTCharsAvail(uart.baseAddr)) {
            rxChar = (char)(UARTCharGetNonBlocking(uart.baseAddr) & 0xFF);
            bool rxDone = (
                    (rxChar == '\r') || (rxChar == '\n') ||
                    (rxChar == 0x1b) || (rxChar == '\0') ||
                    (uart.bufLen == uart.bufMax-1)
            );

            if (rxChar == '\b') {
                if (uart.bufLen > 0) {
                    uart.bufLen--;
                    uart.bufPtr--;
                }
            }
            else if (rxDone) {
                *uart.bufPtr = '\0';
                uart.bufLen++;

                UARTIntDisable(uart.baseAddr, (UART_INT_RX | UART_INT_RT));
                vTaskNotifyGiveFromISR(uart.waitTask, &xHigherPriorityTaskWoken);
            }
            else {
                *uart.bufPtr = rxChar;

                uart.bufPtr++;
                uart.bufLen++;
            }
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
