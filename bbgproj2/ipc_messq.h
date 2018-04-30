<<<<<<< HEAD
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
#include "logger.h"
#include "myusrled.h"

#ifndef ipc_messq_h_
#define ipc_messq_h_

#define DEFAULT_BUF_SIZE   256

extern file_t ipcfile;         //creates a file where queue info is stored, mounted

extern mqd_t ipc_queue;        //queue associated with main thread
extern mqd_t log_queue;

extern struct mq_attr ipc_attr;

extern struct mq_attr ipc_attr;

extern mqd_t ipc_queue;

/*types of ipc messages that are possible*/
typedef enum{
  MSG_NONE, MSG_QUERY, MSG_DATA, MSG_INFO, MSG_TERMINATE, MSG_ERROR, MSG_HB
} message_t;

/*locations messages can be sent to and received from*/
typedef enum{
  IPC_NONE, IPC_LOG, IPC_MAIN, IPC_SOCKET, IPC_USER, IPC_HB
} location_t;

/*struct to define messages passed around to all parts of the system*/
typedef struct ipcmessage {
  char timestamp[10];
  message_t type;                   //message identifier
  comm_t comm_type;
  location_t source;                //where message originates from
  pid_t src_pid;                    //pid of process creating the message
  location_t destination;           //final destination for message
  char payload[DEFAULT_BUF_SIZE];   // message to transmit
} ipcmessage_t;

void ipc_queue_init();
void shuffler_king();
void log_queue_init();

void build_ipc_msg(ipcmessage_t msg_struct, char* ipc_msg);
void decipher_ipc_msg(char* ipc_msg, ipcmessage_t* msg_struct);
void manage_ipc_msg(ipcmessage_t msg, char* log_str);

#endif /* __ipc_messq_h_*/
=======
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
>>>>>>> 44b5fad0908d13310cca4cbae1cdf1b3f726b099
