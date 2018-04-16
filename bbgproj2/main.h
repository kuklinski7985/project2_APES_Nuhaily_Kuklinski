/**
* @file main.h
* @brief main fxn for project1 - APES, globals
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
#include <mqueue.h>
#include "i2c_wrapper.h"
#include "tempsense.h"
#include "temp_ops.h"
#include "light_ops.h"
#include "remote_socket_server.h"
#include "logger/logger.h"

#include "ipc_messq.h"
//#include "myusrled.h"

#define DEFAULT_BUF_SIZE    256

typedef struct input_struct{
  int member1;
} input_struct;

pthread_t tempops_thread;    //creates new pthread
pthread_t lightops_thread;    //creates new pthread
pthread_t log_thread;
pthread_attr_t attr;         //standard attributes for pthread

file_t logfile;
file_t ipcfile;             
file_t tempipcfile;
file_t lightipcfile;

int bizzounce;
mqd_t log_queue;           //queue associated with logger
mqd_t ipc_queue;           //queue associated with main thread
mqd_t temp_ipc_queue;      //queue associated with temp sensor
mqd_t light_ipc_queue;

struct mq_attr ipc_attr;          //attributes struct for ipc queue

void* heartbeat();
void hb_warn(union sigval arg);
void hb_hb_fn(union sigval arg);
