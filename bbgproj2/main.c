/**
* @file main.c
* @brief main fxn for project2 (BBG Board) - APES
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include "main.h"

int main(int argc, char* argv[])
{
  char ipc_queue_buff[DEFAULT_BUF_SIZE];
  
  ipc_queue_init();           //main queue created
  log_queue_init();          //starts the queue fo the logger

  int checking;                    //check value for pthread creation
  input_struct * input1;           //input for pthread,couldnt get to work w/o
  char msg_str[DEFAULT_BUF_SIZE];
  char buf1[DEFAULT_BUF_SIZE];

  char log_filename[DEFAULT_BUF_SIZE];
  ipcmessage_t ipc_msg;

  input1 = (input_struct*)malloc(sizeof(input_struct));
  input1->member1 = 1234;
  pthread_attr_init(&attr);

  checking = pthread_create(&log_thread, &attr, logger, (void*)input1);
  if(checking)
  {
    fprintf(stderr, "Error creating log thread");
    return -1;
  }

  checking = pthread_create(&hb_thread, &attr, heartbeat, (void*)input1);
  if(checking)
  {
    fprintf(stderr, "Error creating heartbeat thread");
    return -1;
  }

  checking = pthread_create(&comm_thread, &attr, commthreadrx, (void*)input1);
  if(checking)
  {
    fprintf(stderr, "Error creating comm rx thread");
    return -1;
  }

  checking = pthread_create(&comm_thread, &attr, loopbackthreadrx, (void*)input1);
  if(checking)
  {
    fprintf(stderr, "Error creating loopback rx thread");
    return -1;
  }

  checking = pthread_create(&terminal_thread, &attr, userterminal, (void*)input1);
  if(checking)
  {
    fprintf(stderr, "Error creating user terminal thread");
    return -1;
  }

  checking = pthread_create(&socket_thread, &attr, serversocket, (void*)input1);
  if(checking)
  {
    fprintf(stderr, "Error creating socket thread");
    return -1;
  }
  
  strcpy(ipc_msg.timestamp, getCurrentTimeStr());
  ipc_msg.type = MSG_INFO;
  ipc_msg.source = IPC_MAIN;
  ipc_msg.destination = IPC_LOG;
  ipc_msg.src_pid = getpid();
  strcpy(ipc_msg.payload, "Main thread initialized.\n");
  build_ipc_msg(ipc_msg, msg_str);
  mq_send(ipc_queue, msg_str, strlen(msg_str), 0);
  memset(msg_str, 0, strlen(msg_str));

  /****using argc to give file name for logger at run time****/
  if(argc > 1)
  {
    strcpy(log_filename, argv[1]);
  }
  else
  {
    strcpy(log_filename, "prj.log");
  }
  
  strcpy(logfile.filename, log_filename);

  if(fileCreate(&logfile) == -1)
  {
    printf("Error creating logfile.\n");
    bizzounce = 1;
  }
 
 /******monitors the main message queue for new messages and distributes accordingly******/
 /******also provides the system heartbeat for the sensors*******/
  mq_getattr(ipc_queue, &ipc_attr);
    //printf("comm rx thread created successfully.\n");
  // Print user terminal menu
  printTerminalMenu();
  printTerminalPrompt();

  while(bizzounce == 0)
  {
    while(ipc_attr.mq_curmsgs > 0)
    { 
	    shuffler_king();
	    mq_getattr(ipc_queue, &ipc_attr);
    }

    if(log_hb_err > 0)
    {
        memset(msg_str, 0, strlen(msg_str));
        strcpy(ipc_msg.timestamp, getCurrentTimeStr());
        ipc_msg.type = ERROR;
        ipc_msg.source = IPC_MAIN;
        ipc_msg.destination = IPC_LOG;
        ipc_msg.src_pid = getpid();
        strcpy(ipc_msg.payload, "");
        strcat(ipc_msg.payload, "Log thread timed out.");
        strcat(ipc_msg.payload, "\n");
        build_ipc_msg(ipc_msg, msg_str);
        mq_send(ipc_queue, msg_str, strlen(msg_str), 0);
        memset(msg_str, 0, strlen(msg_str));
        // blink an LED?
    }
    mq_getattr(ipc_queue, &ipc_attr);
  }

/***joining threads, closing files, and removing queues*****/
  strcpy(ipc_msg.timestamp, getCurrentTimeStr());
  ipc_msg.type = MSG_INFO;
  ipc_msg.source = IPC_MAIN;
  ipc_msg.destination = IPC_LOG;
  ipc_msg.src_pid = getpid();
  strcpy(ipc_msg.payload, "Main thread exiting.\n");
  build_ipc_msg(ipc_msg, msg_str);
  mq_send(ipc_queue, msg_str, strlen(msg_str), 0);

  mq_close(ipc_queue);

  if(mq_unlink("/ipcmain") == -1)
  {
      printf("unlink error: %s\n", strerror(errno));
  }

  pthread_join(socket_thread,NULL);
  
  pthread_join(hb_thread, NULL);

  pthread_join(log_thread, NULL);
  mq_close(log_queue);
  printf("mq_close err: %s\n", strerror(errno));

}

//definition for heartbeat
void* heartbeat()
{
  char msg_str[DEFAULT_BUF_SIZE];
  ipcmessage_t ipc_msg;

  strcpy(ipc_msg.timestamp, getCurrentTimeStr());
  ipc_msg.type = MSG_INFO;
  ipc_msg.source = IPC_HB;
  ipc_msg.destination = IPC_LOG;
  ipc_msg.src_pid = getpid();
  strcpy(ipc_msg.payload, "Heartbeat thread initialized.\n");
  build_ipc_msg(ipc_msg, msg_str);
  mq_send(ipc_queue, msg_str, strlen(msg_str), 0);

  log_hb_count = 0;
  log_hb_err = 0;

  timer_t temp_hb;
  //sets values for timer interval and initial expiration
  struct itimerspec temp_hb_interval;
  //describe the way a process is to be notified about and event
  struct sigevent temp_hb_sig;

  temp_hb_sig.sigev_notify = SIGEV_THREAD;
  temp_hb_sig.sigev_notify_function = hb_warn;
  temp_hb_sig.sigev_value.sival_ptr = &temp_hb;
  temp_hb_sig.sigev_notify_attributes = NULL;

  temp_hb_interval.it_value.tv_sec = 1;
  temp_hb_interval.it_value.tv_nsec = 0;
  temp_hb_interval.it_interval.tv_sec = temp_hb_interval.it_value.tv_sec;
  temp_hb_interval.it_interval.tv_nsec = temp_hb_interval.it_value.tv_nsec;
  timer_create(CLOCK_REALTIME, &temp_hb_sig, &temp_hb);  //creates new timer
  timer_settime(temp_hb, 0, &temp_hb_interval, 0);    //this starts the counter

  while(bizzounce == 0)
  {
    if(log_hb_count > 10)
    {
      printf("Heartbeat thread: error.\n");
      log_hb_err = 1;
    }
  }

  strcpy(ipc_msg.timestamp, getCurrentTimeStr());
  ipc_msg.type = MSG_INFO;
  ipc_msg.source = IPC_HB;
  ipc_msg.destination = IPC_LOG;
  ipc_msg.src_pid = getpid();
  strcpy(ipc_msg.payload, "Heartbeat thread exiting.\n");
  build_ipc_msg(ipc_msg, msg_str);
  mq_send(ipc_queue, msg_str, strlen(msg_str), 0);

}

void hb_warn(union sigval arg)
{
  log_hb_count++;
}

void* userterminal()
{
  // no need for any open / read / write here because the user terminal is defined as UART0
  // infinite while loop checking scanf()
  // if data is found, format it to an ipc message type
  // push it to the ipc queue
  char buf[DEFAULT_BUF_SIZE];
  ipcmessage_t ipc_buf;


  while(bizzounce == 0)
  {
    if(scanf("%s", buf) == -1)
    {
      printf("Error reading from terminal.\n");
      // might be better to form a message and put it on the ipc queue so main can do this display
    }

    else
    {
      switch(buf[0])
      {
        case 'm':
        case 'M':
          // would it be better if main did this display?
          printTerminalMenu();
          break;

        case 'c':
        case 'C':
          // construct an ipc item to send a string over uart1
          strcpy(ipc_buf.timestamp, getCurrentTimeStr() );
          ipc_buf.source = IPC_USER;
          ipc_buf.destination = IPC_UART;
          ipc_buf.type = MSG_QUERY;
          ipc_buf.comm_type = COMM_QUERY;
          ipc_buf.src_pid = getpid();
          strcpy(ipc_buf.payload, "Test string.\n");
          strcpy(buf, "");
          build_ipc_msg(ipc_buf, buf);
          mq_send(ipc_queue, buf, strlen(buf), 0); // send to main and have main transmit over UART
          break;

        case 'd':
        case 'D':
          // construct an ipc item to send a string over uart1
          strcpy(ipc_buf.timestamp, getCurrentTimeStr() );
          ipc_buf.source = IPC_USER;
          ipc_buf.destination = IPC_UART;
          ipc_buf.type = MSG_QUERY;
          ipc_buf.comm_type = COMM_QUERY;
          ipc_buf.src_pid = getpid();
          strcpy(ipc_buf.payload, "Loopback string.\n");
          strcpy(buf, "");
          build_ipc_msg(ipc_buf, buf);
          mq_send(ipc_queue, buf, strlen(buf), 0);
          break;

        default:
          printf("Invalid entry.\n");
          break;
      }
    }
    printTerminalPrompt();
  }
}

void printTerminalMenu()
{
  printf("\nBBG Server Terminal Menu:\n");
  printf("(M) Print this menu\n");
  printf("(C) Send a string to the client at UART1\n");
  printf("(D) Send a string to the client at UART2\n");
}

void printTerminalPrompt()
{
    printf("Enter command (M for menu): ");
}