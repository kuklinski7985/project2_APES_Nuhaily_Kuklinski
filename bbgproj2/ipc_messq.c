/**
* @file ipc_messq.c
* @brief fxn definition for queue creation and use
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include "ipc_messq.h"
#include "comm.h"


extern int num_clients;

extern int log_hb_count;
extern int log_hb_err;
extern int hb_client_count[MAX_UART_CLIENTS];
extern int hb_client_err[MAX_UART_CLIENTS];

/**
 * @brief Parse and process messages pulled from main IPC queue
 * 
 */
void shuffler_king()
{ 
  char ipc_queue_buff[DEFAULT_BUF_SIZE];
  char log_str[DEFAULT_BUF_SIZE];
  char socket_str[DEFAULT_BUF_SIZE];
  ipcmessage_t ipc_msg;
  comm_msg_t comm_msg;

  // Pull item
  mq_receive(ipc_queue, ipc_queue_buff, DEFAULT_BUF_SIZE, NULL);
  decipher_ipc_msg(ipc_queue_buff, &ipc_msg);

  // Determine where item wants to go and process accordingly
  switch(ipc_msg.destination) 
  {
    case(IPC_MAIN): // Items destined for main
      if(ipc_msg.type == MSG_HB)
      {
        switch(ipc_msg.source)
        {
          case IPC_LOG:
            log_hb_count = 0;
            log_hb_err = 0;
            break;              
          default:
            break;
        }
      }

      // if not a hb signal, unpack the rest
      else
      {
        // assuming there's something in the payload that main needs to do something about,
        // e.g. a message from the socket or from the user
        switch(ipc_msg.source)
        {
          case IPC_SOCKET:
          case IPC_UART1:
          case IPC_UART2:
          case IPC_UART4:
          case IPC_UART5:
            if(ipc_msg.type == MSG_INFO)  // info messages from the client threads (typically connection status)
            {
              strcpy(log_str, ipc_msg.timestamp);
              strcat(log_str, "INFO: ");
              strcat(log_str, ipc_msg.payload);
              strcat(log_str, "\n");
              printf("%s", log_str);
              mq_send(log_queue, log_str, strlen(log_str), 0);
            }
            // messages that have come from the socket, check the payload and process according to its contents
            // handle UART messages the same as socket
            // switch on the comm_t message type
            switch(ipc_msg.comm_type)
            {
              // types:
             // COMM_NONE, COMM_QUERY, COMM_DATA, COMM_INFO, COMM_CMD, COMM_ERROR, COMM_HB 
              case COMM_INFO:
                // some status info was received from the client, format, display, and log the payload
                strcpy(log_str, ipc_msg.timestamp);
                strcat(log_str, "INFO: ");
                strcat(log_str, ipc_msg.payload);
                strcat(log_str, "\n");
                printf("%s", log_str);
                mq_send(log_queue, log_str, strlen(log_str), 0);
                break;

              case COMM_DATA:
                // a data packet was received from the TIVA, do something with it
                break;

              case COMM_ERROR:
                // error message received from TIVA, determine what the error is, how to respond to it, log it, display
                strcpy(log_str, ipc_msg.timestamp);
                strcat(log_str, "ERROR FROM CLIENT: ");
                strcat(log_str, ipc_msg.payload);
                strcat(log_str, "\n");
                printf("%s", log_str);
                mq_send(log_queue, log_str, strlen(log_str), 0);
                break;
              
              case COMM_CMD:  // the TIVA does not send commands to the BBG
              case COMM_NONE:
              default:
                break;  // do we need to log something here?
            }
            break;
        }
      }
      break;

        // Items to be pushed to log queue. Display to terminal and send to log
    case IPC_LOG:
      manage_ipc_msg(ipc_msg, log_str);
      mq_send(log_queue, log_str, strlen(log_str), 0);
      break;

    // Items for transmit to a client
    // Designed to behave independent of hardware connection to client- determine destination
    // and write to appropriate interface
    case IPC_SOCKET:
    case IPC_UART1:
    case IPC_UART2:
    case IPC_UART4:
    case IPC_UART5:
    case IPC_LOOPBACK:
      // convert ipc message type to comm message type
      strcpy(comm_msg.timestamp, ipc_msg.timestamp);
      comm_msg.type = ipc_msg.comm_type; // determine type based on packet contents (payload?)
      strcpy(comm_msg.payload, ipc_msg.payload);
      strcpy(socket_str, "");
      build_comm_msg(comm_msg, socket_str);
      switch(ipc_msg.destination)
      {
        case IPC_UART1: // Write to UART directly
          uart_write(uart_client[0], socket_str);
          break;
        case IPC_UART2:
          uart_write(uart_client[1], socket_str);
          break;
        case IPC_UART4:
          uart_write(uart_client[2], socket_str);
          break;
        case IPC_UART5:
          uart_write(uart_client[3], socket_str);
          break;
        case IPC_SOCKET:  // push to socket queue and let socket thread handle
          //mq_send(socket_queue, socket_str, strlen(socket_str), 0);
          break;   
        default:
          break;
      }
      break;

    // General user-use items
    case IPC_USER:
      mq_send(log_queue, log_str, strlen(log_str), 0);
      break;
    
    // Type-less or erroneous items
    case IPC_NONE:
    default:
      //printf("Destination %d not valid\n", ipc_msg.destination);
      break;
  }
}

/**
 * @brief Initialize IPC queue. Requires mount of appropriate mqueue folder
 * 
 */
void ipc_queue_init()
{
  ipc_attr.mq_maxmsg = 255;
  ipc_attr.mq_msgsize = sizeof(char)*DEFAULT_BUF_SIZE;
  ipc_attr.mq_flags = 0;

  ipc_queue = mq_open("/ipc_main", O_CREAT | O_RDWR, 0666, &ipc_attr);
}

/**
 * @brief Initialize log queue. Requires mount of appropriate mqueue folder
 * 
 */
void log_queue_init()
{
  struct mq_attr log_attr;
  log_attr.mq_maxmsg = 255;
  log_attr.mq_msgsize = sizeof(char)*DEFAULT_BUF_SIZE;
  log_attr.mq_flags = 0;

  log_queue = mq_open("/log", O_CREAT | O_RDWR, 0666, &log_attr);
  //printf("Log queue init status: %s\n", strerror(errno));
}

/**
 * @brief Translate from ipc message struct type to string for queue transmit
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
  char tmp2[16];
  
  // extract timestamp
  for(i=0, j=0; ipc_msg[i] != '\n' && ipc_msg[i] != '\0'; i++, j++)
  {
    msg_struct->timestamp[j] = ipc_msg[i];
  }
  msg_struct->timestamp[j] = '\0';
  
  // determine message type
  strcpy(tmp1, "");
  for(i++, j=0; ipc_msg[i] != '\n' && ipc_msg[i] != '\0'; i++, j++)
  {
    tmp1[j] = ipc_msg[i];
  }
  msg_struct->type = (message_t)atoi(tmp1);

  // determine source process
  strcpy(tmp1, "");
  for(i++, j=0; ipc_msg[i] != '\n' && ipc_msg[i] != '\0'; i++, j++)
  {
    tmp1[j] = ipc_msg[i];
  }
  msg_struct->source = (location_t)atoi(tmp1);

  // source PID
  for(i++, j=0; ipc_msg[i] != '\n' && ipc_msg[i] != '\0'; i++, j++)
  {
    tmp2[j] = ipc_msg[i];
  }
  msg_struct->src_pid = (pid_t)atoi(tmp2);
  
  strcpy(tmp1, "");
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
  msg_struct->payload[j] = '\0';  // mqueue seems to require a null terminator
                                  // as the receive function doesn't append one

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

  sprintf(tmp, "%d", (int)msg_struct.src_pid);
  strcat(ipc_msg, tmp);
  strcat(ipc_msg, "\n");

  sprintf(tmp, "%d", msg_struct.destination);
  strcat(ipc_msg, tmp);
  strcat(ipc_msg, "\n");

  strcat(ipc_msg, msg_struct.payload);
  strcat(ipc_msg, "\n");
}

/**
 * @brief Determine appropriate formatting for IPC message depending on type
 * 
 * @param msg 
 * @param log_str 
 */
void manage_ipc_msg(ipcmessage_t msg, char* log_str)
{
  char tmp[DEFAULT_BUF_SIZE];
  char loglevel[16];
  char sourceid[64];
  
  // Determine type of incoming message and process depending on type, source, and destination
  switch(msg.type)
  {
    case(MSG_DATA):
      strcpy(loglevel, "DATA: ");
      if(msg.source == IPC_NONE)//IPC_LIGHT) // reassigned temporarily
      {
        sprintf(tmp, "%s%s%s%s%s.\n", msg.timestamp, loglevel, "Light sensor reads: ", msg.payload, " lux");  
      }
      else if(msg.source == IPC_MAIN)//IPC_TEMP)
      {
        sprintf(tmp, "%s%s%s%s.\n", msg.timestamp, loglevel, "Temp sensor reads: ", msg.payload);
      }
      break;

    case(MSG_INFO):
      strcpy(loglevel, "INFO: "); 
      switch(msg.source)
      {
        case(IPC_LOG):
          strcpy(sourceid, "Logger message: ");
          break;
        case(IPC_MAIN):
          strcpy(sourceid, "Main message: ");
          break;
        case(IPC_HB):
          strcpy(sourceid, "Heartbeat thread: ");
          break;
        default:
          strcpy(sourceid, "err (sourceid) ");
          break;
      }
      //usr_led_toggle(1, 0);
      snprintf(tmp, DEFAULT_BUF_SIZE, "%s%s%s%s\n", msg.timestamp, loglevel, \
        sourceid, msg.payload);
      break;

      case(MSG_ERROR):
        strcpy(loglevel, "ERROR: ");
        switch(msg.source)
        {
          case(IPC_LOG):
            strcpy(sourceid, "Logger message: ");
            break;
          case(IPC_MAIN):
            strcpy(sourceid, "Main message: ");
            break;
          case(IPC_HB):
            strcpy(sourceid, "Heartbeat thread: ");
            break;
          default:
            strcpy(sourceid, "err (sourceid) ");
            break;
        }
        usr_led_toggle(1, 1);
        snprintf(tmp, DEFAULT_BUF_SIZE, "%s%s%s%s\n", msg.timestamp, loglevel, \
          sourceid, msg.payload);
        break;

    default:
      break;
  }
  strcpy(log_str, tmp);
  
  // disabled immediate display of log messages for this program
  /*
  if(msg.type != IPC_USER)
  {
    printf("%s", log_str);
  }*/

}

