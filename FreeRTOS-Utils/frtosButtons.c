/*
 * switches.c
 *
 *  Created on: May 23, 2020
 *      Author: m009b
 */

#include <stdbool.h>
#include <stdint.h>

#include "buttons.h"

#include "TivaWare/driverlib/debug.h"
#include "frtosButtons.h"

/*
 * FreeRTOS button driver.
 * This driver uses the two buttons in the Tiva C series 1294XL board to create events for FreeRTOS tasks.
 * Driver uses an EventGroup that tasks can latch onto and block using two separate functions:
 *      - xGetButtonEvent       - in order to establish a max wait time for a button event to occur.
 *      - xWaitForButtonEvent   - in order to indefinitely wait for a switch event to occur.
 * Tasks use the buttonMask config to specify which button event to look for.
 * Use BTN1, BTN2, or EITHER_BUTTON macros defined in frtosButtons.h as values for buttonMask.
 * An "AND" configuration is currently not supported.
 */

static EventGroupHandle_t sg_SwitchGroupHandle;

static TaskHandle_t ButtonTask;

void vButtonHandler();
void buttonManageTask(void *pvParam);


/**
 * @brief Initializes the Button's GPIO module & the FreeRTOS task/event Group other tasks attach to.
 */
void vButtonsInit() {

    buttonsInit(BUTTON_CONFIG, vButtonHandler);

    // FreeRTOS driver init
    sg_SwitchGroupHandle = xEventGroupCreate();
    ASSERT(sg_SwitchGroupHandle != NULL);

    xTaskCreate(buttonManageTask, "BTNS", 128, NULL, configMAX_PRIORITIES, &ButtonTask);
}

/**
 * @brief   Get button event
 * @param   buttonMask:
 *              Bit-mask of which button event to look for.
 *              Bit 0 & 1 are used for button 1 & 2, respectively.
 *              Bit mask is used as an OR configuration.
 * @param   waitTimeMS: Max time to wait for a button Event in miliseconds.
 * @return  A bitmask of what button event occurred (will not return a value that isn't compatible with the buttonMask parameter passed).
 * @details Tasks that call this function will block until a button event occurs (or a timeout has occurred).
 */
EventBits_t xGetButtonEvent(uint8_t buttonMask, TickType_t waitTimeMS) {
    EventBits_t swEvent = xEventGroupWaitBits(sg_SwitchGroupHandle, buttonMask, pdTRUE, pdFALSE, waitTimeMS/portTICK_PERIOD_MS);

    return (swEvent & buttonMask);
}

/**
 * @brief   Wait for button event.
 * @param   buttonMask:
 *              Bit-mask of which button event to look for.
 *              Bit 0 & 1 are used for button 1 & 2, respectively.
 *              Bit mask is used as an OR configuration.
 * @return  A bitmask of what button event occurred (will not return a value that isn't compatible with the buttonMask parameter passed).
 * @details Tasks that call this function will block until a button event occurs. Task will block indefinitely if no button event occurs.
 */
EventBits_t xWaitForButtonEvent(EventBits_t buttonMask) {
    EventBits_t swEvent = 0;

    while (!(swEvent & buttonMask)) {
        swEvent = xEventGroupWaitBits(sg_SwitchGroupHandle, buttonMask, pdTRUE, pdFALSE, portMAX_DELAY);
    }

    return swEvent;
}

/**
 * @brief   Interrupt handler for the GPIO port module the buttons are connected to.
 *
 * @details Handler wakes up the Button management task & performs tasks to debounce the button signal.
 */
void vButtonHandler() {

    uint32_t status = resetButtonInterrupt();

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xResult = xTaskNotifyFromISR(ButtonTask, status, eSetBits, &xHigherPriorityTaskWoken);

    if (xResult == pdPASS) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/**
 * @brief   Button Management Task.
 * @details This task is in charge of performing software debouncing of the button signal.
 *          Due to it being required for signal debouncing,
 *          it also handles setting event group bits to awake tasks awaiting an event.
 */
void buttonManageTask(void *pvParam) {
    uint32_t xButtonIntStatus, xButtonStatus = 0;
    BaseType_t xNotificationRet = 0;

    while (1) {
        xNotificationRet = xTaskNotifyWait(EITHER_BUTTON, EITHER_BUTTON, &xButtonIntStatus, portMAX_DELAY);

        if (xNotificationRet == pdPASS) {
            /*
             * Allow a short delay to occur to allow the button press to stabilize.
             */
            vTaskDelay(DEBOUNCE_DELAY_MS);

            /*
             * Read button status after delay based on what pin triggered the interrupt.
             * If it returns a non-zero number then accept it as an actual button press.
             */
            xButtonStatus = getButtonStatus(xButtonIntStatus);
            if (xButtonStatus) {
                xEventGroupSetBits(sg_SwitchGroupHandle, xButtonStatus);    // Wake up tasks awaiting

                /*
                 * Wait for the button to be "unpressed".
                 * This will avoid false-positive glitches with the unbounced button once it's unpressed.
                 */
                while (xButtonStatus) {
                    vTaskDelay(DEBOUNCE_DELAY_MS);
                    xButtonStatus = getButtonStatus(xButtonIntStatus);
                }

                resetButtonInterrupt();
            }
        }
    }
}
