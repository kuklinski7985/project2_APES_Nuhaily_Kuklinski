/**
* @file remote_socket_server.h
* @brief fxns prototypes for remote socket initialization and usage
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include <arpa/inet.h>

#define MAX_NUM_CLIENTS  1024
#define MAX_READ_BUFFER_SIZE 256

#ifndef server_socket_h_
#define server_socket_h_

/**
 *@brief creates, initializes, and is the callback function for the server thread
 *
 *@param "VOID" nothing
 *
 *@return VOID
 */

void* serversocket();

typedef struct struct_mess{
    char message[256];
    float float_val;
    int int_val;
}struct_mess_t;

#endif /*__server_socket_h_*/




