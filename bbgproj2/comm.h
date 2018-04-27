/**
* @file comm.h
* @brief fxn definition for queue creation and use
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
#include "prj2types.h"
#include "ipc_messq.h"

#ifndef COMM_H_
#define COMM_H_

#define DEFAULT_BUFFER_SIZE     256



typedef int uart_t; // uart port file descriptor type

// we don't really need a location type do we? if the server is sending a message it's obviously going to the client
// if the client receives a message it was obiviously from the server
// I guess if we have multiple clients we could have the server know which client it was by identifying what port it received it from

//uart_t user_terminal;
uart_t uart_client;//[3]; // eventually expand to array of clients somehow
uart_t loopback_client;

void decipher_comm_msg(char* comm_msg, comm_msg_t* msg_struct);
void build_comm_msg(comm_msg_t msg_struct, char* comm_msg);

void decipher_comm_data(data_t comm_data, char* payload);
void build_comm_data(char* payload, comm_msg_t comm_data);

void* commthreadrx();
void* loopbackthreadrx();

// message format (if it contains sensor data): <type>\n<timestamp>\n<data_t>|<payload>

#endif