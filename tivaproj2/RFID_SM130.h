/**
* @file RFID_SM130.h
* @brief prototype functions for the SM130 rfid reader
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

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


#define RFID_RECV_BUFF_LENGTH    32

#ifndef RFID_SM130_h_
#define RFID_SM130_h_

//extern QueueHandle_t ipc_msg_queue;

/*******************variables********************/
extern uint8_t rfid_handler_exit_flag;
char rfid_data_recv[RFID_RECV_BUFF_LENGTH];


/******************functions********************/

void RFID_reset();

void RFID_seek();

void RFID_antPWR();

/*if using this function, you will not know if the changes have taken affect
 * until you change the UART baud rate to the new overall rate
 * and then reset the system
 */
void RFID_setbaud(uint32_t newBaud);

void UARTRFID_send(char * RFID_buffer, uint32_t byteCount);

#endif /*_RFID_SM130_h_*/
