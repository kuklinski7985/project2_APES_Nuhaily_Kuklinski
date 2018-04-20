/**
* @file remote_socket_server.h
* @brief fxns prototypes for remote socket initialization and usage
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <mqueue.h>

#ifndef server_socket_h_
#define server_socket_h_

/**
 *@brief initializes remote socket and keeps it open for data transfers, created in a thread and kept open until bizzounce
 *
 *@param "VOID" nothing
 *
 *@return VOID
 */

int client_socket_init(int port);

typedef struct struct_mess{
    char message[256];
    float float_val;
    int int_val;
}struct_mess_t;

#endif /*__server_socket_h_*/




