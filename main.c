

/**
 * main.c
 */
#include <stdbool.h>
#include <stdint.h>
#include "sysinit.h"
#include "leds.h"
#include "frtosButtons.h"
#include "frtosUARTstdio.h"

#include "FreeRTOS.h"
#include "FreeRTOS/include/task.h"

TaskHandle_t testTask;

void vTestTask(void *pvParam) {
    EventBits_t switchEvent = 0;

    uint8_t state = 0;

    uint8_t ledTable[] =
    {
     LEDS_OFF,
     LED1_MASK,
     (LED1_MASK | LED2_MASK),
     (LED1_MASK | LED2_MASK | LED3_MASK),
     LEDS_ON,
     (LED2_MASK | LED3_MASK | LED4_MASK),
     (LED3_MASK | LED4_MASK),
     LED4_MASK
    };

    while (1) {
        switchEvent = xWaitForButtonEvent(EITHER_BUTTON);

        if (switchEvent & BTN1) {
            state++;
        }
        else if (switchEvent & BTN2) {
            state--;
        }

        ledsWrite(ALL_LEDS, ledTable[(state & LED4_MASK-1)]);
    }
}

void vButtonTask(void *pvParam) {
    EventBits_t switchEvent = 0;

    while (1) {
        switchEvent = xWaitForButtonEvent(EITHER_BUTTON);

        if (switchEvent & BTN1) {
            vUARTprintf("Button 1 pressed!\n");
        }

        if (switchEvent & BTN2) {
            vUARTprintf("Button 2 pressed!\n");
        }
    }
}

int main(void)
{
    sysinit();

    vUartStdioInit(9600, SYSTEM_CLOCK);

    xTaskCreate(vButtonTask, "BTN TASK", 128, NULL, tskIDLE_PRIORITY+1, &testTask);



    vTaskStartScheduler();

    while (1) {
    }

	return 0;
}

/*  ASSERT() Error function
 *
 *  failed ASSERTS() from driverlib/debug.h are executed in this function
 */
void __error__(char *pcFilename, uint32_t ui32Line)
{
    // Place a breakpoint here to capture errors until logging routine is finished
    while (1)
    {
    }
}

