/*
 * main.h
 *
 *  Created on: Apr 8, 2018
 *      Author: Adam Nuhaily
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"


// FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"
#include "task.h"
#include "timers.h"
#include "FreeRTOS_sockets.h"

// Project 2 includes
#include "ipc_message.h"

//socket, MAC, and TCP/IP
#include "networkinterface.h"

//RFID sensor
#include "RFID_SM130.h"

#ifndef MAIN_H_
#define MAIN_H_

#define CLOCK_120MHZ    120000000U
#define SYSTEM_CLOCK    CLOCK_120MHZ
#define ULONG_MAX       0xFFFFFFFF

// Task Notify signals
#define TOGGLE_LED              32
#define LOG_STRING              64
#define QUEUE_ELEMENT_SIZE_MAX  256
#define QUEUE_LENGTH_MAX        8
#define DEFAULT_BUFFER_SIZE     256
#define UARTBAUDRATE            19200

void ConfigureUART();
void vPrintTerminalMenu(void);
void vPrintTerminalPrompt(void);
int xProcessTerminalInput(char* input);
int xProcessHBInput(char* input);

// LED Tasks
void vMasterTask(void* pvParameters);
void vUARTRxTerminalTask(void* pvParameters);
void vGPIOTask(void* pvParameters);
void vHeartbeatTask(void* pvParameters);
void vHBTimerCallback(void* pvParameters);

// Interrupt handlers
void UARTTerminalIntHandler(void);

void UART_RFID_Handler(void);
void UARTRFID_send(char * RFID_buffer, uint32_t byteCount);
void vSocketTask(void *pvParameters);

int xInitThreads(void);
void vRFIDTask(void *pvParameters);

//value for the data read from rfid
extern char rfid_data_recv[32];
uint8_t rfid_handlder_exit_flag;


// Timer callbacks
void vHBTimerCallback(void* pvParameters);

extern void decipher_ipc_msg(char* ipc_msg, ipcmessage_t* msg_struct);

typedef uint32_t heartbeat_t;

#endif /* MAIN_H_ */
