/**
* @file ipc_messq.c
* @brief fxn definition for queue creation and use
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include "ipc_message.h"

/**
 * @brief Translate from ipc message string to ipc message struct type
 * 
 * @param ipc_msg 
 * @param msg_struct 
 * Messages are formatted with \n separating each field (delimiters).
 */
void decipher_ipc_msg(char* ipc_msg, ipcmessage_t* msg_struct)
{
  int i=0;
  int j=0;
  char tmp1[1];
  //char tmp2[TASKNAME_SIZE];

  // extract timestamp
  for(i=0, j=0; ipc_msg[i] != '\n' && ipc_msg[i] != '\0'; i++, j++)
  {
    msg_struct->timestamp[j] = ipc_msg[i];
  }
  msg_struct->timestamp[j] = '\0';
  
  // determine message type
  for(i++, j=0; ipc_msg[i] != '\n' && ipc_msg[i] != '\0'; i++, j++)
  {
    tmp1[j] = ipc_msg[i];
  }
  msg_struct->type = (message_t)atoi(tmp1);

  // determine source process
  for(i++, j=0; ipc_msg[i] != '\n' && ipc_msg[i] != '\0'; i++, j++)
  {
    tmp1[j] = ipc_msg[i];
  }
  msg_struct->source = (location_t)atoi(tmp1);

 /* // source PID
  for(i++, j=0; ipc_msg[i] != '\n' && ipc_msg[i] != '\0'; i++, j++)
  {
      tmp1[j] = ipc_msg[i];
  }
  msg_struct->src_handle = (TaskHandle_t)atoi(tmp1);
*/
  // destination process
  for(i++, j=0; ipc_msg[i] != '\n' && ipc_msg[i] != '\0'; i++, j++)
  {
    tmp1[j] = ipc_msg[i];
  }
  msg_struct->destination = (location_t)atoi(tmp1);

  // message payload (terminated by null char)
  for(i++, j=0; ipc_msg[i] != '\n' && ipc_msg[i] != '\0'; i++, j++)
  {
    msg_struct->payload[j] = ipc_msg[i];
  }
  msg_struct->payload[j] = '\0';  // do FreeRTOS queues append a null terminator? linux doesn't- needs testing

}

/**
 * @brief Translate from string to ipc message type for parsing and processing
 * 
 * @param msg_struct 
 * @param ipc_msg 
 */
void build_ipc_msg(ipcmessage_t msg_struct, char* ipc_msg)
{
  char tmp[DEFAULT_BUF_SIZE];

  strcpy(ipc_msg, msg_struct.timestamp);
  strcat(ipc_msg, "\n");

  sprintf(tmp, "%d", msg_struct.type);
  strcat(ipc_msg, tmp);
  strcat(ipc_msg, "\n");

  sprintf(tmp, "%d", msg_struct.source);
  strcat(ipc_msg, tmp);
  strcat(ipc_msg, "\n");

 /* sprintf(tmp, "%d", msg_struct.src_handle);
  strcat(ipc_msg, tmp);
  strcat(ipc_msg, "\n");
*/
  sprintf(tmp, "%d", msg_struct.destination);
  strcat(ipc_msg, tmp);
  strcat(ipc_msg, "\n");

  strcat(ipc_msg, msg_struct.payload);
  strcat(ipc_msg, "\n");
}


