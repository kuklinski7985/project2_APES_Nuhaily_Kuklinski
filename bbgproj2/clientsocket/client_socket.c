/**
* @file socketclient.c
* @brief fxn external program that creates a socket in order to obtain sensor information, client side
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/


#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdint.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <mqueue.h>

#include "client_socket.h"

//#define PORT 8090


//add input variables to init IP address and port, return value of sock_handle, and move send/receving messages initializers to another function
int client_socket_init(int port)
{
  struct sockaddr_in sock_addr;    //struct that contains the port and Ip addr
  int sock_handle;                 //assigned socket tracking value
  int valread;                     //return value for read

  char rec_message[256];
  char send_message[256] = "MESSGAES! to server from client!\n";

  sock_handle = socket(AF_INET, SOCK_STREAM, 0);
  if(sock_handle < 0)
    {
      printf("Socket NOT Created\n");
    }

  sock_addr.sin_family = AF_INET;
  sock_addr.sin_port = htons(port);

  if(inet_pton(AF_INET, "127.0.0.1", &sock_addr.sin_addr)<=0)
    {
      printf("Invalid Address\n");
    }
  
  if(connect(sock_handle, (struct sockaddr*)&sock_addr, sizeof(sock_addr))<0)
    {
      printf("connection to server FAILED!\n");
    }
  
  return sock_handle;
}

  /*send(sock_handle, send_message, 256,0);

  valread = read(sock_handle, rec_message, 256);
  printf("Message Received from Server: %s\n",rec_message);
}*/