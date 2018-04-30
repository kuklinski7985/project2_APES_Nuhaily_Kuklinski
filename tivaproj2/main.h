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
#include "driverlib/systick.h"



// FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"
#include "task.h"
#include "timers.h"
#include "FreeRTOS_sockets.h"
#include "semphr.h"

// Project 2 includes
#include "ipc_message.h"

//socket, MAC, and TCP/IP
#include "networkinterface.h"

//RFID sensor
#include "RFID_SM130.h"

//Camera
#include "myCamera_simple.h"


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
#define UARTBAUDRATE            115200

//configures all uart's that are necessary for operations
void ConfigureUART();

//user interface input into the system, not operational
void vPrintTerminalMenu(void);

//also user interface input
void vPrintTerminalPrompt(void);

//processes data that is inputted from the the user and does something
int xProcessTerminalInput(char* input);

//when a heartbeat expires, this function processes which thread should be terminated
int xProcessHBInput(char* input);

//gathers and distributes IPC messages for the system
void vMasterTask(void* pvParameters);

//processes incomming information for UART0
void vUARTRxTerminalTask(void* pvParameters);

//controls the 4 usr leds
void vGPIOTask(void* pvParameters);

//task creates the heartbeat timer and send a notification message to main when connections are lost
void vHeartbeatTask(void* pvParameters);

//callback function for the heartbeat timer
void vHBTimerCallback(void* pvParameters);

//task which contains functionality for VC0706 TTL camera
void vCameraTask(void* pvParameters);

//Task to process UART logger
void vTerminalLogging(void* pvParameters);

// Interrupt handlers
void UARTTerminalIntHandler(void);

//RFID Functions
//UART interrupt handler for the RFID sensor
void UART_RFID_Handler(void);

//task to process incoming data from the RFID
void vRFIDTask(void *pvParameters);

//starts all threads
int xInitThreads(void);


//value for the data read from rfid
extern char rfid_data_recv[RFID_RECV_BUFF_LENGTH];
extern char camera_data_recv[CAMERA_RECV_BUFF_LENGTH];
uint8_t rfid_handler_exit_flag;
uint8_t camera_handler_exit_flag;


// Timer callbacks
void vHBTimerCallback(void* pvParameters);

extern void decipher_ipc_msg(char* ipc_msg, ipcmessage_t* msg_struct);

typedef uint32_t heartbeat_t;

void vTimeStampCallBack( void* pvParameters );




#endif /* MAIN_H_ */
