/**
* @file comm.c
* @brief fxn definition for queue creation and use
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comm.h"

void decipher_comm_msg(char* comm_msg, comm_msg_t* msg_struct)
{
  int i=0;
  int j=0;
  char tmp1[1];
  char tmp2[16];

  // extract timestamp
  for(i=0, j=0; comm_msg[i] != '\n' && comm_msg[i] != '\0'; i++, j++)
  {
    msg_struct->timestamp[j] = comm_msg[i];
  }
  msg_struct->timestamp[j] = '\0';
  
  // determine message type
  for(i++, j=0; comm_msg[i] != '\n' && comm_msg[i] != '\0'; i++, j++)
  {
    tmp1[j] = comm_msg[i];
  }

  // Make sure we're not calling aoti with a null string - I think this is the 
  //  cause of the seg faults
  if(tmp1[0] != '\0')
  {
    msg_struct->type = (comm_t)atoi(tmp1);
  }

  // message payload (terminated by null char)
  for(i++, j=0; comm_msg[i] != '\n' && comm_msg[i] != '\0'; i++, j++)
  {
    msg_struct->payload[j] = comm_msg[i];
  }
  msg_struct->payload[j] = '\0';  // mqueue seems to require a null terminator
                                  // as the receive function doesn't append one

}

void build_comm_msg(comm_msg_t msg_struct, char* comm_msg)
{
  char tmp[DEFAULT_BUFFER_SIZE];

  strcpy(comm_msg, msg_struct.timestamp);
  strcat(comm_msg, "\n");

  sprintf(tmp, "%d", msg_struct.type);
  strcat(comm_msg, tmp);
  strcat(comm_msg, "\n");

  strcat(comm_msg, msg_struct.payload);
  strcat(comm_msg, "\n");
}

void decipher_comm_data(data_t comm_data, char* payload)
{
  int i=0;
  int j=0;
  char tmp1[1];
  char tmp2[16];
  char tmp3[16];

  // determine sensor type
  for(i++, j=0; payload[i] != '\n' && payload[i] != '\0'; i++, j++)
  {
    tmp1[j] = payload[i];
  }

  // Make sure we're not calling aoti with a null string - I think this is the 
  //  cause of the seg faults
  if(tmp1[0] != '\0')
  {
    comm_data->sensor_type = (sensor_t)atoi(tmp1);
  }

  // determine sensor type
  for(i++, j=0; payload[i] != '\n' && payload[i] != '\0'; i++, j++)
  {
    tmp2[j] = payload[i];
  }

  // Make sure we're not calling aoti with a null string - I think this is the 
  //  cause of the seg faults
  if(tmp2[0] != '\0')
  {
    comm_data->sensorid = (int)atoi(tmp2);
  }

  // determine sensor type
  for(i++, j=0; payload[i] != '\n' && payload[i] != '\0'; i++, j++)
  {
    tmp3[j] = payload[i];
  }

  // Make sure we're not calling aoti with a null string - I think this is the 
  //  cause of the seg faults
  if(tmp3[0] != '\0')
  {
    comm_data->data = (uint32_t)atoi(tmp3);
  }

}

/*
 *
 */

void build_comm_data(char* payload, data_t* comm_data)
{
  char tmp[DEFAULT_BUFFER_SIZE];

  strcpy(payload, comm_data.timestamp);
  strcat(payload, "\n");

  sprintf(tmp, "%d", comm_data.type);
  strcat(payload, tmp);
  strcat(payload, "\n");

  strcat(payload, comm_data.payload);
  strcat(payload, "\n");
}

/*
manage_comm_msg(comm_msg_t comm_msg)
{
  // determine message type
  // handle in a big switch statement by pushing correct actions to queue
  // socket queue should interpret messages into an IPC message struct type
}
*/

