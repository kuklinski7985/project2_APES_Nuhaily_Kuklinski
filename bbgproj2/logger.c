/**
 * @brief Logger thread and file & system I/O functions
 * 
 * @file logger.c
 * @author Adam Nuhaily and Andrew Kuklinski
 * @date 2018-03-17
 */
#include "logger.h"

pthread_mutex_t log_mutex;
pthread_mutex_t time_mutex;
pthread_mutex_t sprintf_mutex;

extern int bizzounce;   // Exit signal
extern mqd_t log_queue;
extern mqd_t ipc_queue;

extern file_t logfile;

extern int log_hb_count;
extern int log_hb_err;

/**
 * @brief Logger thread handler function
 * 
 * @return void* 
 */
void* logger()
{
  // initialize log queue
  struct mq_attr log_attr;

  char queue_buf[DEFAULT_BUF_SIZE];
  unsigned int prio;
  ipcmessage_t ipc_msg;
  char msg_str[DEFAULT_BUF_SIZE];

  while(bizzounce == 0)
  {
    mq_getattr(log_queue, &log_attr);  //keeps the thread alive to process signals and timer requests
    while(log_attr.mq_curmsgs > 0)
    {
      // Loop and inspect log queue for new items to pull
      // When present, pull and insert into logfile
      mq_receive(log_queue, queue_buf, DEFAULT_BUF_SIZE, &prio);
      writeLogStr(&logfile, queue_buf);
      mq_getattr(log_queue, &log_attr);
      memset(queue_buf, 0, strlen(queue_buf));
    }
    log_hb_count = 0;
    log_hb_err = 0;
  }
  
  // Desire to exit, close logffile
  fileClose(&logfile);
}

/**
 * @brief Intended to be a sigevent handler for mq_notify, no longer used
 * 
 */
static void logger_handler()
{
  char queue_buf[DEFAULT_BUF_SIZE];
  unsigned int prio;
  // file ops:
  // open file
  // read from queue
  mq_receive(log_queue, queue_buf, DEFAULT_BUF_SIZE, &prio);

  // add to file
  writeLogStr(&logfile, queue_buf);

}

/**
 * @brief Handler function to exit log thread gracefully
 * 
 */
void log_exit()
{
  printf("exit signal received : log thread!\n\n");
}

/**
 * @brief Write log_str to logfile, thread-safe
 * 
 * @param logfile 
 * @param log_str 
 */
void writeLogStr(file_t* logfile, char* log_str)
{
  pthread_mutex_lock(&log_mutex);
  fileWrite(logfile, log_str);  // logfile is already a pointer no need to
                                      // pass address-of
  pthread_mutex_unlock(&log_mutex);
}

/**
 *  @brief Return current time formatted appropriately for logging and display
 *  @return string containing current time as "hh:mm:dd > "
 *  Thread-safe
 */
char* getCurrentTimeStr()
{
  //localtime() converts epoch time to local time, rtn as struct tm
  time_t current_time;
  struct tm* current_time_tm;
  char ascii_int_buf[8];
  static char time_str[13]; // must be static to be returnable

  pthread_mutex_lock(&time_mutex);  // Be thread-safe so other requests don't
                                    //  overwrite the calling thread's time

  //time (&current_time);
  current_time = time(0);
  current_time_tm = localtime(&current_time);

  // Format time string properly and return it
  thread_sprintf(ascii_int_buf, current_time_tm->tm_hour, "%02lu");
  strcpy(time_str, ascii_int_buf);
  strcat(time_str, ":");
  thread_sprintf(ascii_int_buf, current_time_tm->tm_min, "%02lu");
  strcat(time_str, ascii_int_buf);
  strcat(time_str, ":");
  thread_sprintf(ascii_int_buf, current_time_tm->tm_sec, "%02lu");
  strcat(time_str, ascii_int_buf);

  strcat(time_str, " > ");

  pthread_mutex_unlock(&time_mutex);

  return time_str;
}

/**
 *  @brief Thread-safe sprintf for converting int to ascii
 *  @param format format of output
 *  @return return status of function
 *
 */
int8_t thread_sprintf(char* rtn_ascii, long lng, char format[])
{
  pthread_mutex_lock(&sprintf_mutex);
  sprintf(rtn_ascii, format, lng);    // Convert int to ascii and store
  pthread_mutex_unlock(&sprintf_mutex);
  return 0;
}
