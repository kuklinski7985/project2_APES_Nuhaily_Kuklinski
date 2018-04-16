/**
* @file ipc_messq.c
* @brief fxn definition for queue creation and use
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include "ipc_messq.h"

extern int temp_hb_count;
extern int temp_hb_err;
extern int light_hb_count;
extern int light_hb_err;
extern int log_hb_count;
extern int log_hb_err;
extern int hb_hb_count;
extern int hb_hb_err;

/**
 * @brief Parse and process messages pulled from main IPC queue
 * 
 */
void shuffler_king()
{ 

  char ipc_queue_buff[DEFAULT_BUF_SIZE];
  char log_str[DEFAULT_BUF_SIZE];
  ipcmessage_t ipc_msg;

  // Pull item
  mq_receive(ipc_queue, ipc_queue_buff, DEFAULT_BUF_SIZE, NULL);
  decipher_ipc_msg(ipc_queue_buff, &ipc_msg);

  // Determine where item wants to go and process accordingly
  switch(ipc_msg.destination) 
  {
      case(IPC_MAIN): // Items destined for main only, probably just display
        if(ipc_msg.type == HEARTBEAT) // no longer in use
        {
          switch(ipc_msg.source)
          {
            case(IPC_TEMP):
              temp_hb_count = 0;
              temp_hb_err = 0;
              break;
            case(IPC_LIGHT):
              light_hb_count = 0;
              light_hb_err = 0;
              break;
            case(IPC_LOG):
              log_hb_count = 0;
              log_hb_err = 0;
              break;              
            default:
              break;
          }
        }
        break;

        // Items to be pushed to log queue. Display to terminal and send to log
      case(IPC_LOG):
        manage_ipc_msg(ipc_msg, log_str);
        mq_send(log_queue, log_str, strlen(log_str), 0);
        break;

      // General user-use items
      case(IPC_USER):
        mq_send(log_queue, log_str, strlen(log_str), 0);
        break;

      // Items to be sent to temperature sensor task
      case(IPC_TEMP):
        mq_send(temp_ipc_queue, ipc_queue_buff,strlen(ipc_queue_buff),0);
        break;

      // Items to be sent to light sensor task
      case(IPC_LIGHT):
        mq_send(light_ipc_queue, ipc_queue_buff,strlen(ipc_queue_buff),0);
        break;
      
      // Type-less or erroneous items
      case(IPC_NONE):
      default:
        printf("Destination %d not valid\n", ipc_msg.destination);
  }
}

/**
 * @brief Previously intended to be temperature sensor-specific, now not used
 * 
 */
void shuffler_mini_temp()
{
  char temp_ipc_queue_buff[DEFAULT_BUF_SIZE];
  
  if (mq_notify(temp_ipc_queue, &sigevent_temp_ipc_notify) == -1)
    {
      printf("mq_notify error: %s\n", strerror(errno));
    }

  mq_getattr(temp_ipc_queue, &temp_ipc_attr);
  while(temp_ipc_attr.mq_curmsgs > 0)
    {
      mq_receive(temp_ipc_queue, temp_ipc_queue_buff, DEFAULT_BUF_SIZE, NULL);
      mq_getattr(temp_ipc_queue, &temp_ipc_attr);
      //printf("remaining on temp queue %ld\n",temp_ipc_attr.mq_curmsgs);
      //printf("Temp Q read message: %s\n",temp_ipc_queue_buff);
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
  //printf("IPC queue init status: %s\n", strerror(errno));

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
 * @brief Initialize temp sensor queue. Requires mount of mqueue folder.
 * 
 */
void temp_ipc_queue_init()
{
  temp_ipc_attr.mq_maxmsg = 255;
  temp_ipc_attr.mq_msgsize = sizeof(char)*DEFAULT_BUF_SIZE;
  temp_ipc_attr.mq_flags = 0;

  temp_ipc_queue = mq_open("/ipctemperature", O_CREAT | O_RDWR, 0666, &temp_ipc_attr);

  sigevent_temp_ipc_notify.sigev_notify = SIGEV_THREAD;
  sigevent_temp_ipc_notify.sigev_notify_function = shuffler_mini_temp;
  sigevent_temp_ipc_notify.sigev_notify_attributes = NULL;
  sigevent_temp_ipc_notify.sigev_value.sival_ptr = NULL;
  if (mq_notify(temp_ipc_queue, &sigevent_temp_ipc_notify) == -1)
  {
    printf("mq_notify error: %s\n", strerror(errno));
  }
  
}

/**
 * @brief Initialize light sensor queue. Requires mount of mqueue folder.
 * 
 */
void light_ipc_queue_init()
{
  //struct mq_attr light_ipc_attr;

  light_ipc_attr.mq_maxmsg = 255;
  light_ipc_attr.mq_msgsize = sizeof(char)*DEFAULT_BUF_SIZE;
  light_ipc_attr.mq_flags = 0;

  light_ipc_queue = mq_open("/ipclight", O_CREAT | O_RDWR, 0666, &temp_ipc_attr);

  sigevent_light_ipc_notify.sigev_notify = SIGEV_THREAD;
  sigevent_light_ipc_notify.sigev_notify_function = shuffler_mini_light;
  sigevent_light_ipc_notify.sigev_notify_attributes = NULL;
  sigevent_light_ipc_notify.sigev_value.sival_ptr = NULL;
  if (mq_notify(light_ipc_queue, &sigevent_light_ipc_notify) == -1)
    {
      printf("mq_notify error: %s\n", strerror(errno));
    }
  
}

/**
 * @brief Previously intended to be light sensor specific handler. Not used.
 * 
 */
void shuffler_mini_light()
{
  char light_ipc_queue_buff[DEFAULT_BUF_SIZE];
  printf("entering light shuffler\n");
  
  if (mq_notify(light_ipc_queue, &sigevent_light_ipc_notify) == -1)
    {
      printf("mq_notify error: %s\n", strerror(errno));
    }
  
  mq_getattr(light_ipc_queue, &light_ipc_attr);
  while(light_ipc_attr.mq_curmsgs > 0)
    {
      mq_receive(light_ipc_queue, light_ipc_queue_buff, DEFAULT_BUF_SIZE, NULL);
      mq_getattr(light_ipc_queue, &light_ipc_attr);
      //printf("remaining on temp queue %ld\n",light_ipc_attr.mq_curmsgs);
      printf("Light Q read message: %s\n",light_ipc_queue_buff);
      sleep(1);
    }
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

  // source PID
  for(i++, j=0; ipc_msg[i] != '\n' && ipc_msg[i] != '\0'; i++, j++)
  {
    tmp2[j] = ipc_msg[i];
  }
  msg_struct->src_pid = (pid_t)atoi(tmp2);

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

  switch(msg.type)
  {
    case(DATA):
      strcpy(loglevel, "DATA: ");
      if(msg.source == IPC_LIGHT)
      {
        //printf("Light sensor reads: %s lumens.\n", msg.payload);
        sprintf(tmp, "%s%s%s%s%s.\n", msg.timestamp, loglevel, "Light sensor reads: ", msg.payload, " lux");
    
      }
      else if(msg.source == IPC_TEMP)
      {
        sprintf(tmp, "%s%s%s%s.\n", msg.timestamp, loglevel, "Temp sensor reads: ", msg.payload);
      }
      break;

    case(INFO):
      strcpy(loglevel, "INFO: "); 
      switch(msg.source)
      {
        case(IPC_LIGHT):
          strcpy(sourceid, "Light sensor message: ");
          break;
        case(IPC_TEMP):
          strcpy(sourceid, "Temp sensor message: ");
          break;
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
      snprintf(tmp, DEFAULT_BUF_SIZE, "%s%s%s%s\n", msg.timestamp, loglevel, sourceid, msg.payload);
      break;

      case(MSG_ERROR):
        strcpy(loglevel, "ERROR: ");
        switch(msg.source)
        {
          case(IPC_LIGHT):
            strcpy(sourceid, "Light sensor message: ");
            break;
          case(IPC_TEMP):
            strcpy(sourceid, "Temp sensor message: ");
            break;
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
        snprintf(tmp, DEFAULT_BUF_SIZE, "%s%s%s%s\n", msg.timestamp, loglevel, sourceid, msg.payload);
        break;

    default:
      break;
  }
  strcpy(log_str, tmp);
  if(msg.type != IPC_USER)
  {
    printf("%s", log_str);
  }

}
