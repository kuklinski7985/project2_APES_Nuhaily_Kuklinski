/**
* @file ipc_messq.h
* @brief fxn prototypes for queue creation and use
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

#ifndef ipc_message_h_
#define ipc_message_h_

#define DEFAULT_BUF_SIZE    256
#define TASKNAME_SIZE       64

extern uint8_t tiva_sec;
extern uint8_t tiva_min;
extern SemaphoreHandle_t xtimestamp_sema;
extern char timeString[9];

// types of messages that are possible
typedef enum{
  MSG_QUERY, MSG_DATA, MSG_INFO, MSG_TERMINATE, MSG_ERROR, MSG_HB
} message_t;

// locations messages can be sent to and received from
typedef enum{
  TASK_NONE, TASK_MAIN, TASK_SOCKET, TASK_UART, TASK_TERMINAL, TASK_HB, TASK_GPIO, TASK_LOGGING, TASK_RFID
} location_t;


// struct to define messages passed around to all parts of the system
typedef struct ipcmessage {
  char timestamp[10];
  message_t type;                   // message identifier
  location_t source;                // where message originates from
  //TaskHandle_t src_handle;   // handle of process creating the message
  location_t destination;           // final destination for message
  char payload[DEFAULT_BUF_SIZE];   // message to transmit
} ipcmessage_t;

void build_ipc_msg(ipcmessage_t msg_struct, char* ipc_msg);
void decipher_ipc_msg(char* ipc_msg, ipcmessage_t* msg_struct);
void manage_ipc_msg(ipcmessage_t msg, char* log_str);
void getTimeStamp(ipcmessage_t *ipc_mess);

#endif /* __ipc_message_h_*/
