/**
* @file main.c
* @brief main fxn for project1 - APES
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include "main.h"

pthread_t tempops_thread;    //creates new pthread for the temperature sensor
pthread_t lightops_thread;    //creates new pthread for the light sensor
pthread_t log_thread;         //new thread for logger
pthread_t socket_thread;      //thread for the remote socket
pthread_t hb_thread;          //heartbeat sensor thread

pthread_attr_t attr;         //standard attributes for pthread
file_t logfile;             
file_t ipcfile;
file_t tempipcfile;

int bizzounce;              //global variable to exit all threads and close
mqd_t log_queue;           //queue associated with logger
mqd_t ipc_queue;           //queue associated with main thread
mqd_t temp_ipc_queue;      //queue associated with temp sensor

struct mq_attr ipc_attr;          //attributes struct for ipc queue


extern float light_previous;  //global for use in socket, light values
extern char * temp_previous;  //global for use in socket, temp values

int temp_hb_count;
int temp_hb_err;
int light_hb_count;
int light_hb_err;
int log_hb_count;
int log_hb_err;

int main(int argc, char* argv[])
{
  char ipc_queue_buff[DEFAULT_BUF_SIZE];
  
  ipc_queue_init();           //main queue created
  log_queue_init();          //starts the queue fo the logger
  temp_ipc_queue_init();      //temp sensor queue created
  light_ipc_queue_init();     //light sensor queue created

  int checking;                    //check value for pthread creation
  input_struct * input1;           //input for pthread,couldnt get to work w/o
  char msg_str[DEFAULT_BUF_SIZE];
  char buf1[DEFAULT_BUF_SIZE];

  char log_filename[DEFAULT_BUF_SIZE];
  ipcmessage_t ipc_msg;
 // remote_socket_server_init();

  input1 = (input_struct*)malloc(sizeof(input_struct));
  input1->member1 = 1234;
  pthread_attr_init(&attr);

  checking = pthread_create(&log_thread, &attr, logger, (void*)input1);
  if(checking)
  {
    fprintf(stderr, "Error creating log thread");
    return -1;
  }

  checking = pthread_create(&tempops_thread, &attr, temp_ops,(void*)input1);
  if(checking)
  {
    fprintf(stderr, "Error creating temp_ops thread");
    return -1;
  }

  checking = pthread_create(&lightops_thread, &attr, light_ops, (void*)input1);
  if(checking)
  {
    fprintf(stderr, "Error creating light_ops thread");
    return -1;
  }


 checking = pthread_create(&socket_thread, &attr, remote_socket_server_init,(void*)input1);

  checking = pthread_create(&hb_thread, &attr, heartbeat, (void*)input1);
  if(checking)
  {
    fprintf(stderr, "Error creating heartbeat thread");
    return -1;
  }
  
  strcpy(ipc_msg.timestamp, getCurrentTimeStr());
  ipc_msg.type = INFO;
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
  while(bizzounce == 0)
  {
    while(ipc_attr.mq_curmsgs > 0)
    { 
	    shuffler_king();
	    mq_getattr(ipc_queue, &ipc_attr);
    }
    if(temp_hb_err > 0 || light_hb_err > 0 || log_hb_err > 0)
    {
        memset(msg_str, 0, strlen(msg_str));
        strcpy(ipc_msg.timestamp, getCurrentTimeStr());
        ipc_msg.type = ERROR;
        ipc_msg.source = IPC_MAIN;
        ipc_msg.destination = IPC_LOG;
        ipc_msg.src_pid = getpid();
        strcpy(ipc_msg.payload, "");
        if(temp_hb_err > 0)
        {
          strcat(ipc_msg.payload, "Temperature sensor thread timed out.");
        }
        if(light_hb_err > 0)
        {
          strcat(ipc_msg.payload, "Light sensor thread timed out.");
        }
        if(log_hb_err > 0)
        {
          strcat(ipc_msg.payload, "Log thread timed out.");
        }
        strcat(ipc_msg.payload, "\n");
        build_ipc_msg(ipc_msg, msg_str);
        mq_send(ipc_queue, msg_str, strlen(msg_str), 0);
        memset(msg_str, 0, strlen(msg_str));
    }
    mq_getattr(ipc_queue, &ipc_attr);
  }

/***joining threads, closing files, and removing queues*****/
  strcpy(ipc_msg.timestamp, getCurrentTimeStr());
  ipc_msg.type = INFO;
  ipc_msg.source = IPC_TEMP;
  ipc_msg.destination = IPC_LOG;
  ipc_msg.src_pid = getpid();
  strcpy(ipc_msg.payload, "Main thread exiting.\n");
  build_ipc_msg(ipc_msg, msg_str);
  mq_send(ipc_queue, msg_str, strlen(msg_str), 0);

  mq_close(ipc_queue);
  mq_close(temp_ipc_queue);
  mq_close(light_ipc_queue);
  if(mq_unlink("/ipcmain") == -1)
    {
      	printf("unlink error: %s\n", strerror(errno));
    }

  if(mq_unlink("/ipctemperature") == -1)
    {
      printf("unlink error: %s\n", strerror(errno));
    }

  pthread_join(tempops_thread, NULL);

  pthread_join(lightops_thread, NULL);

  pthread_join(log_thread, NULL);

  pthread_join(socket_thread,NULL);
  
  pthread_join(hb_thread, NULL);

  pthread_join(log_thread, NULL);
  mq_close(log_queue);
  printf("mq_close err: %s\n", strerror(errno));

  pthread_join(log_thread, NULL);

}
//definition for heartbeat
void* heartbeat()
{
  char msg_str[DEFAULT_BUF_SIZE];
  ipcmessage_t ipc_msg;

  strcpy(ipc_msg.timestamp, getCurrentTimeStr());
  ipc_msg.type = INFO;
  ipc_msg.source = IPC_HB;
  ipc_msg.destination = IPC_LOG;
  ipc_msg.src_pid = getpid();
  strcpy(ipc_msg.payload, "Heartbeat thread initialized.\n");
  build_ipc_msg(ipc_msg, msg_str);
  mq_send(ipc_queue, msg_str, strlen(msg_str), 0);

  temp_hb_count = 0;
  temp_hb_err = 0;
  light_hb_count = 0;
  light_hb_err = 0;
  log_hb_count = 0;
  log_hb_err = 0;

  timer_t temp_hb;
  //sets values for timer interval and initial expiration
  struct itimerspec temp_hb_interval;
  //descibe the way a process is to be notified about and event
  struct sigevent temp_hb_sig;

  temp_hb_sig.sigev_notify = SIGEV_THREAD;
  temp_hb_sig.sigev_notify_function = hb_warn;
  temp_hb_sig.sigev_value.sival_ptr = &temp_hb;
  temp_hb_sig.sigev_notify_attributes = NULL;

  temp_hb_interval.it_value.tv_sec = 1;
  temp_hb_interval.it_value.tv_nsec = 0;//10000000000;
  temp_hb_interval.it_interval.tv_sec = temp_hb_interval.it_value.tv_sec;//0;
  temp_hb_interval.it_interval.tv_nsec = temp_hb_interval.it_value.tv_nsec;//0;
  timer_create(CLOCK_REALTIME, &temp_hb_sig, &temp_hb);  //creates new timer
  timer_settime(temp_hb, 0, &temp_hb_interval, 0);    //this starts the counter

  while(bizzounce == 0)
  {
    if(temp_hb_count > 10)
    {
      printf("Heartbeat thread: error.\n");
      temp_hb_err = 1;
    }
    if(light_hb_count > 10)
    {
      printf("Heartbeat thread: error.\n");
      light_hb_err = 1;
    }
    if(log_hb_count > 10)
    {
      printf("Heartbeat thread: error.\n");
      log_hb_err = 1;
    }
  }

  strcpy(ipc_msg.timestamp, getCurrentTimeStr());
  ipc_msg.type = INFO;
  ipc_msg.source = IPC_TEMP;
  ipc_msg.destination = IPC_LOG;
  ipc_msg.src_pid = getpid();
  strcpy(ipc_msg.payload, "Heartbeat thread exiting.\n");
  build_ipc_msg(ipc_msg, msg_str);
  mq_send(ipc_queue, msg_str, strlen(msg_str), 0);

}

void hb_warn(union sigval arg)
{
  temp_hb_count++;
  light_hb_count++;
  log_hb_count++;
}
