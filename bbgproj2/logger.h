/**
 *  @file logger.h
 *  @author Adam Nuhaily and Andrew Kuklinski
 *  @date 11 Mar 2018
 *  @brief hw3 header file
 *  This source file implements the multithreaded program described in the
 *  homework 3 assignment document.
 */

#ifndef _LOGGER_H
#define _LOGGER_H

#define _GNU_SOURCE

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
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
#include "sync_fileio.h"
#include "../ipc_messq.h"

#define DEFAULT_BUF_SIZE     256

typedef enum
{
  ERROR,
  MESSAGE,
  TEMP,
  LIGHT
} log_type_t;

void* logger();
static void logger_handler();
void log_exit();
void writeLogStr(file_t* logfile, char* log_str);
char* getCurrentTimeStr();
int8_t thread_sprintf(char* rtn_ascii, long lng, char format[]);

#endif
