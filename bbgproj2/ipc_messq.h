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
#include <pthread.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <mqueue.h>
#include <errno.h>
#include "logger/logger.h"
#include "temp_ops.h"
#include "myusrled.h"

#ifndef ipc_messq_h_
#define ipc_messq_h_

#define DEFAULT_BUF_SIZE   256

extern file_t ipcfile;         //creates a file where queue info is stored, mounted
extern file_t tempipcfile;     //creates file where temperature queue info is stored, mounted

extern mqd_t ipc_queue;        //queue associated with main thread
extern mqd_t temp_ipc_queue;    //queue associated with temp sensor
extern mqd_t light_ipc_queue;
extern mqd_t log_queue;

extern struct mq_attr ipc_attr;
struct mq_attr temp_ipc_attr;
struct mq_attr light_ipc_attr;
struct sigevent sigevent_temp_ipc_notify;
struct sigevent sigevent_light_ipc_notify;

extern struct mq_attr ipc_attr;

extern mqd_t ipc_queue;

/*types of messages that are possible*/
typedef enum{
  QUERY, DATA, INFO, TERMINATE, MSG_ERROR, HEARTBEAT
} message_t;

/*locations messages can be sent to and received from*/
typedef enum{
  IPC_NONE, IPC_LOG, IPC_TEMP, IPC_LIGHT, IPC_MAIN, IPC_SOCKET, IPC_USER, IPC_HB
} location_t;


/*struct to define messages passed around to all parts of the system*/
typedef struct ipcmessage {
  char timestamp[10];
  message_t type;                   //message identifier
  location_t source;                //where message originates from
  pid_t src_pid;                    //pid of process creating the message
  location_t destination;           //final destination for message
  char payload[DEFAULT_BUF_SIZE];   // message to transmit
  temp_unit_t units_temp;
} ipcmessage_t;

void ipc_queue_init();
void shuffler_king();
void log_queue_init();
void temp_ipc_queue_init();

void shuffler_mini_temp();

void light_ipc_queue_init();

void shuffler_mini_light();

void build_ipc_msg(ipcmessage_t msg_struct, char* ipc_msg);
void decipher_ipc_msg(char* ipc_msg, ipcmessage_t* msg_struct);
void manage_ipc_msg(ipcmessage_t msg, char* log_str);

#endif /* __ipc_messq_h_*/
