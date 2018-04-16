/**
* @file processtreemodule.c
* @brief TIVA TM4C1294NCPDT Blinky project
* @author Andrew Kuklinski
* @date 04-11-2018
**/

/* This project will blink usrLed1 at 2Hz and usrLed2 at 4Hz while printing off UART status messages.
 * It includes the use of TivaWare and FreeRTOS.  Its purpose is to be used as a "copy-able" file for
 * future projects.
 *
 */


#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

uint32_t SysClkFreq;
static uint8_t led1Value = 0;
static uint8_t led2Value = 0;

void my_UARTprintString(char * input_string);

void vledBlink1(void *pvParameters);
void vledBlink2(void *pvParameters);

static void led1CallBackFxn (TimerHandle_t xTimer);
static void led2CallBackFxn (TimerHandle_t xTimer);

#define led1_TIMER_PERIOD       pdMS_TO_TICKS(500)
#define led2_TIMER_PERIOD       pdMS_TO_TICKS(250)

int main(void)
{


    /*sets the clock freq
     *(external crystal freq | use external crystal | PLL source | PLL VCO to 320Mhz), desired freq
     */
    SysClkFreq = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_320), 120000000);

    //USRLED 1 and 2 are connected to port N, turns it on
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    //turning on URAT0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0|GPIO_PIN_1);
    //setting pins for RX / TX
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTConfigSetExpClk(UART0_BASE, SysClkFreq, 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    my_UARTprintString("Sample Project to Make sure things work");
    UARTCharPut(UART0_BASE, 10);    //carriage return
    UARTCharPut(UART0_BASE, 13);    //new line
    //configures Port N, pins 0 and 1 as output

    //writes 0x0 to both pins turning them off
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0x00);

    BaseType_t led1 = xTaskCreate(vledBlink1, "ledBlink1", 1000, NULL, 1, NULL);
    if(led1 == pdFAIL)
    {
        my_UARTprintString("Task ledBlink1 has failed");
    }

    BaseType_t led2 = xTaskCreate(vledBlink2, "ledBlink2", 1000, NULL, 1, NULL);
    if(led2 == pdFAIL)
    {
        my_UARTprintString("Task ledBlink2 has failed");
    }

    vTaskStartScheduler();
    while(1)
    {

    }
}

void my_UARTprintString(char * input_string)
{
    uint8_t string_length = strlen(input_string);
    uint8_t i;
    for(i=0; i<string_length; i++)
    {
        UARTCharPut(UART0_BASE, input_string[i]);
    }

}

void vledBlink1(void *pvParameters)
{
    TimerHandle_t led1Timer;
    BaseType_t led1TimerStarted;
    led1Timer = xTimerCreate("led1timer",led1_TIMER_PERIOD,pdTRUE,0,led1CallBackFxn);
    if(led1Timer != NULL)
    {
        led1TimerStarted = xTimerStart(led1Timer,0);
        if(led1TimerStarted == pdPASS)
        {
            my_UARTprintString("Timer 1 - Initialized!");
            UARTCharPut(UART0_BASE, 13);
            UARTCharPut(UART0_BASE, 10);
        }
    }
    for(;;){}
}

void vledBlink2(void *pvParameters)
{
    TimerHandle_t led2Timer;
    BaseType_t led2TimerStarted;
    led2Timer = xTimerCreate("led2timer",led2_TIMER_PERIOD,pdTRUE,0,led2CallBackFxn);
    if(led2Timer != NULL)
    {
        led2TimerStarted = xTimerStart(led2Timer,0);
        if(led2TimerStarted == pdPASS)
        {
            my_UARTprintString("Timer 2 - Initialized!");
            UARTCharPut(UART0_BASE, 13);
            UARTCharPut(UART0_BASE, 10);
        }
    }
    for(;;){}
}

static void led1CallBackFxn (TimerHandle_t xTimer)
{
    char led1mess[64];
    static uint16_t led1counter = 0;
    led1Value = !led1Value;
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, led1Value);
    led1counter++;
    sprintf(led1mess, "Led1: BLINK BLINK! | %d",led1counter);
    my_UARTprintString(led1mess);
    UARTCharPut(UART0_BASE, 13);
    UARTCharPut(UART0_BASE, 10);
}

static void led2CallBackFxn (TimerHandle_t xTimer)
{
    char led2mess[64];
    static uint16_t led2counter = 0;
    led2Value = (!led2Value);
    if(led2Value != 0)
    {
        led2Value++;
    }
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, led2Value);
    led2counter++;
    sprintf(led2mess, "Led2: BLINK! | %d",led2counter);
    my_UARTprintString(led2mess);
    UARTCharPut(UART0_BASE, 13);
    UARTCharPut(UART0_BASE, 10);
}








