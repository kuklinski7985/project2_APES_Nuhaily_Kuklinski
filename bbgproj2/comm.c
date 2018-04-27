/**
* @file comm.c
* @brief fxn definition for queue creation and use
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
* 
* Used http://man7.org/linux/man-pages/man3/tcsetattr.3p.html as reference
* http://man7.org/linux/man-pages/man3/cfsetispeed.3p.html
http://man7.org/linux/man-pages/man3/termios.3.html
* Used https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
*  as reference
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include "comm.h"

extern int bizzounce;

int init_comm()
{
  // initialize UART for inter-board communication
  struct termios term_attr;
  uart_client = open("/dev/ttyO1", O_RDWR | O_NOCTTY | O_SYNC);

  tcgetattr(uart_client, &term_attr);
  // do an error check on the above

  // set terminal baud rates
//  cfsetispeed(&term_attr, B115200);
//  cfsetospeed(&term_attr, B115200);

  cfsetispeed(&term_attr, B9600);
  cfsetospeed(&term_attr, B9600);

  // set non-blocking
  term_attr.c_cc[VMIN] = 0;
  term_attr.c_cc[VTIME] = 10;  // set 1sec timeout (10 deciseconds per struct termios)

  term_attr.c_iflag &= ~IGNBRK;         // disable break processing
        term_attr.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        term_attr.c_oflag = 0;                // no remapping, no delays
        term_attr.c_cc[VMIN]  = 0;            // read doesn't block
        

        term_attr.c_iflag =  IXOFF; // shut off xon/xoff ctrl

        term_attr.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        term_attr.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        term_attr.c_cflag &= ~CSTOPB;
        term_attr.c_cflag &= ~CRTSCTS;

  // set baud rate etc
  tcsetattr(uart_client, TCSANOW, &term_attr);
  // do an error check on above
  
  // setup loopback rx
  loopback_client = open("/dev/ttyO2", O_RDWR | O_NOCTTY | O_SYNC);

  tcgetattr(loopback_client, &term_attr);
  // do an error check on the above

  // set terminal baud rates
//  cfsetispeed(&term_attr, B115200);
//  cfsetospeed(&term_attr, B115200);

  cfsetispeed(&term_attr, B9600);
  cfsetospeed(&term_attr, B9600);

  // set non-blocking
  term_attr.c_cc[VMIN] = 0;
  term_attr.c_cc[VTIME] = 10;  // set 1sec timeout (10 deciseconds per struct termios)

  term_attr.c_iflag &= ~IGNBRK;         // disable break processing
        term_attr.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        term_attr.c_oflag = 0;                // no remapping, no delays
        term_attr.c_cc[VMIN]  = 0;            // read doesn't block
        

        term_attr.c_iflag =  IXOFF; // shut off xon/xoff ctrl

        term_attr.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        term_attr.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        term_attr.c_cflag &= ~CSTOPB;
        term_attr.c_cflag &= ~CRTSCTS;

  // set baud rate etc
  tcsetattr(loopback_client, TCSANOW, &term_attr);
  // do an error check on above
  

  return 0;
}

void* loopbackthreadrx()
{
  char msg_buf[DEFAULT_BUF_SIZE];

  while(bizzounce == 0)
  {
    read(loopback_client, msg_buf, DEFAULT_BUF_SIZE);
    printf("Message received: %s\n", msg_buf);
  }
}

void* commthreadrx()
{
  // initialize comm (uart rx from client)
  // start a while loop to monitor input from uart1 (inter-board comm)
  char msg_buf[DEFAULT_BUF_SIZE]; // may need to be bigger to accomodate large image transfer chunks
  comm_msg_t msg_struct;
  ipcmessage_t ipc_struct;
  
  init_comm();
//  printf("entered commthreadrx.\n");

  while(bizzounce == 0)
  {
    printf("bizzounce: %d\n", bizzounce); // try a loopback from one UART to another
    read(uart_client, msg_buf, DEFAULT_BUF_SIZE);
    printf("!");
    if(strlen(msg_buf) > 0)
    {
      // process from comm type to ipc message type
      printf("Received from client: %s\n", msg_buf);
      decipher_comm_msg(msg_buf, &msg_struct);
      strcpy(ipc_struct.timestamp, msg_struct.timestamp);
      // types:
      // COMM_NONE, COMM_QUERY, COMM_DATA, COMM_INFO, COMM_CMD, COMM_ERROR, COMM_HB 
      ipc_struct.comm_type = msg_struct.type;
      strcpy(ipc_struct.payload, msg_struct.payload);
      strcpy(msg_buf, "");  // clear the buffer so we can reuse it
      
      // now build the surrounding IPC struct attributes relevant to a message coming from the TIVA
      ipc_struct.source = IPC_UART;
      ipc_struct.destination = IPC_MAIN;
      ipc_struct.type = IPC_NONE; // we need to determine the type based on what's in the payload I think?
                            // no, let main determine what to do, just pass the payload and don't switch on type
      ipc_struct.src_pid = getpid();

      // build IPC message string from assembled IPC message struct
      build_ipc_msg(ipc_struct, msg_buf);

      // put on ipc queue
      mq_send(ipc_queue, msg_buf, strlen(msg_buf), 0);
    }
    // no need for else statement? nothing was read
  }  
}

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
    comm_data.sensor_type = (sensor_t)atoi(tmp1);
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
    comm_data.sensorid = (int)atoi(tmp2);
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
    comm_data.data = (uint32_t)atoi(tmp3);
  }

}

/*
 *
 */

void build_comm_data(char* payload, comm_msg_t comm_data)
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

