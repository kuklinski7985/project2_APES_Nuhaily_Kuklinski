/**
* @file server_socket.c
* @brief definitions for server socket
* @author Andrew Kuklinski and Adam Nuhaily
* @date 04/29/2018
**/

#include "server_socket.h"

/**
 * @brief Socket thread handler function
 */
void* serversocket()
{
  int valread;
  int sock_handle;      //creates and initializes the server side socket
  int new_request_connection;  //fd for newly connected socket, available for transfer
  int port1 = 8090;     
  int i;
  int new_activity;   //value of ready descriptors that select returns
  int connected_clients_fd[MAX_NUM_CLIENTS];
  struct sockaddr_in addr;
  int opt = 1;
  int addrlen = sizeof(addr);
  fd_set readfds;
  char send_mess[MAX_READ_BUFFER_SIZE];
  char recv_message[MAX_READ_BUFFER_SIZE];
  //struct_mess_t recv_message;
  int flag = 0;

  for(i=0;i<MAX_READ_BUFFER_SIZE;i++)
  {
    recv_message[i] = '\0';
  }
  
  //printf("Server Initialization Starting\n");
  
  //socket returns file descriptor for the new socket, -1 error
  if((sock_handle = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    printf("Socket Allocation Failed\n");
  }

  if(setsockopt(sock_handle, SOL_SOCKET, (SO_REUSEADDR | SO_REUSEPORT), &opt, sizeof(opt)))
  {
      printf("Error with Setsockopt\n");
  }

//setting connection attributes
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port1);

  if(bind(sock_handle, (struct sockaddr*)&addr,sizeof(addr))<0)
    {
      printf("error on bind\n");
       printf("bind errno %s\n", strerror(errno));
    }

  //printf("Listening mode for server\n");
  listen(sock_handle,5);

  //initializing connected clients array to zero
  for(i=0; i < MAX_NUM_CLIENTS; i++)
  {
    connected_clients_fd[i] = 0;
  }

  //adds the main socket descriptor to the watch list
  //printf("Socket Created using handle %d\n",sock_handle);
  /*this will watch for multiple connections from clients over a socket*/
  while(1)
  {
    FD_ZERO(&readfds);
    FD_SET(sock_handle, &readfds);

    for(i=0;i<MAX_NUM_CLIENTS;i++)
    {
      if(connected_clients_fd[i] > 0)
      {
        FD_SET(connected_clients_fd[i], &readfds);
        printf("clients watching %d\n",connected_clients_fd[i]);
      } 
    }
    //printf("Waiting for new activity...\n");
    new_activity = select((MAX_NUM_CLIENTS+1), &readfds, NULL, NULL, NULL);
    if(new_activity < 0)
    {
      printf("Select Fxn ERROR: %s\n", strerror(errno));
    }
    
    if(FD_ISSET(sock_handle,&readfds))
    {
      printf("New activity detected on socket.\n");
      if((new_request_connection = accept(sock_handle, (struct sockaddr*) &addr, &addrlen)) <0)
      {
        printf("new connection failed");
      }
      printf("New Connection Accepted: FD | ip | port : %d | %s | %d\n",new_request_connection, inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));

      i = 0;
      flag = 0;

      while(flag==0)
      {
        if(connected_clients_fd[i]==0)
        {
          connected_clients_fd[i] = new_request_connection;
          FD_SET(connected_clients_fd[i],&readfds);
          printf("New client FD:%d has been added\n",connected_clients_fd[i]);
          flag = 1;
        }
        else{
          i++;
        }
        if(i==MAX_NUM_CLIENTS)
        {
          flag = 1;
          printf("New client could not be added immediately\n");
        }
      }
      FD_CLR(sock_handle,&readfds);
    }

    for(i=0;i<MAX_NUM_CLIENTS;i++)
    {
      if(FD_ISSET(connected_clients_fd[i],&readfds))
      {
        valread = read(connected_clients_fd[i],recv_message,256);
        printf("message sent to server from client: %s\n",recv_message);

        if(valread == 0)
        {
          close(connected_clients_fd[i]);
          connected_clients_fd[i] = 0;
          printf("Connection closed, %d\n",connected_clients_fd[i]);
        }
        //recv_message[valread] = '\0';
        //send(connected_clients_fd[i],recv_message,strlen(recv_message),0);
      }
    }
    for(i=0;i<MAX_READ_BUFFER_SIZE;i++)
    {
      recv_message[i] = '\0';
    }
  } 
}