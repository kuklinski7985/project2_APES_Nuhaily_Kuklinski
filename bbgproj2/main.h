/**
* @file main.h
* @brief main fxn for project2 - APES, globals
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <mqueue.h>
#include "logger.h"
#include "ipc_messq.h"
#include "myusrled.h"
#include "server_socket.h"
#include "comm.h" 

#define DEFAULT_BUF_SIZE    256

typedef struct input_struct{
  int member1;
} input_struct;

pthread_t log_thread;
pthread_t socket_thread;      //thread for the remote socket
pthread_t hb_thread;          //heartbeat sensor thread
pthread_attr_t attr;         //standard attributes for pthread

file_t logfile;
file_t ipcfile;

int bizzounce;
mqd_t log_queue;           //queue associated with logger
mqd_t ipc_queue;           //queue associated with main thread

struct mq_attr ipc_attr;          //attributes struct for ipc queue

int log_hb_count;
int log_hb_err;

void* heartbeat();
void hb_warn(union sigval arg);
void hb_hb_fn(union sigval arg);
