/**
* @file ipc_messq.h
* @brief fxn prototypes for queue creation and use
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include <stddef.h>
//#include <stdio.h>
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
#include <pthread.h>

#include "logger.h"
#include "myusrled.h"
#include "prj2types.h"

#ifndef ipc_messq_h_
#define ipc_messq_h_

#define DEFAULT_BUF_SIZE   256

extern file_t ipcfile;         //creates a file where queue info is stored, mounted

extern mqd_t ipc_queue;        //queue associated with main thread
extern mqd_t log_queue;

extern struct mq_attr ipc_attr;

extern struct mq_attr ipc_attr;

extern mqd_t ipc_queue;

void ipc_queue_init();
void shuffler_king();
void log_queue_init();

void build_ipc_msg(ipcmessage_t msg_struct, char* ipc_msg);
void decipher_ipc_msg(char* ipc_msg, ipcmessage_t* msg_struct);
void manage_ipc_msg(ipcmessage_t msg, char* log_str);

#endif /* __ipc_messq_h_*/
