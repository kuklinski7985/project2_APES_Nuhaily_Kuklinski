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


#include "comm.h"

extern int bizzounce;

extern int hb_client_count[MAX_UART_CLIENTS];
extern int hb_client_err[MAX_UART_CLIENTS];

/**
 *  @brief Initialize UART ports for clients
 */
int init_comm()
{
  // initialize UARTs 1 through 4 for inter-board communication
  struct termios term_attr;
  struct termios uart_attr;
  char uart_portname_base[32];
  char uart_portname[32];
  char c[1];
  char ipc_string[DEFAULT_BUF_SIZE];
  ipcmessage_t ipc_struct;

  strcpy(uart_portname_base, "/dev/ttyO");

 /* if(fd_terminal = open("/dev/ttyO0", O_RDWR | O_NOCTTY | O_NDELAY) < 0)
  {
    printf("Error opening UART0.\n");
  }
    
  memset(&term_attr, 0, sizeof(term_attr));

  cfsetispeed(&term_attr, B9600);
  cfsetospeed(&term_attr, B9600);

  // set blocking
  term_attr.c_cc[VMIN] = 1;
  term_attr.c_cc[VTIME] = 10;  // set 1sec timeout (10 deciseconds per struct termios)

  // used stackoverflow post and termios man page as reference for these setup items
  term_attr.c_cflag = (term_attr.c_cflag & ~CSIZE) | CS8;     // 8-bit chars

  term_attr.c_iflag &= ~IGNBRK;         // disable break processing
  term_attr.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
  term_attr.c_oflag = 0;                // no remapping, no delays

  term_attr.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

  term_attr.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
  term_attr.c_cflag &= ~(PARENB | PARODD);      // shut off parity
  term_attr.c_cflag |= 0;
  term_attr.c_cflag &= ~CSTOPB;
  term_attr.c_cflag &= ~CRTSCTS;

  // set baud rate etc
  tcsetattr(fd_terminal, TCSANOW, &term_attr);*/

  // Open four UART clients and log + display open status return
  if(uart_client[0] = open("/dev/ttyO1", O_RDWR | O_NOCTTY | O_NDELAY) < 0)
  {
    printf("Error opening UART1.\r\n");
  }
  else
  {
    strcpy(ipc_struct.timestamp, getCurrentTimeStr() );
    ipc_struct.source = IPC_UART1;
    ipc_struct.destination = IPC_MAIN;
    ipc_struct.type = MSG_INFO;
    ipc_struct.comm_type = COMM_NONE;
    ipc_struct.src_pid = (pid_t)getpid();
    strcpy(ipc_struct.payload, "UART1 reference opened successfully.\r\n");
    build_ipc_msg(ipc_struct, ipc_string);
    mq_send(ipc_queue, ipc_string, strlen(ipc_string), 0);
  }

  if(uart_client[1] = open("/dev/ttyO2", O_RDWR | O_NOCTTY | O_NDELAY) < 0)
  {
    printf("Error opening UART2.\r\n");
  }   
  else
  {
    strcpy(ipc_struct.timestamp, getCurrentTimeStr() );
    ipc_struct.source = IPC_UART2;
    ipc_struct.destination = IPC_MAIN;
    ipc_struct.type = MSG_INFO;
    ipc_struct.comm_type = COMM_NONE;
    ipc_struct.src_pid = (pid_t)getpid();
    strcpy(ipc_struct.payload, "UART2 reference opened successfully.\r\n");
    build_ipc_msg(ipc_struct, ipc_string);
    mq_send(ipc_queue, ipc_string, strlen(ipc_string), 0);
  }

  if(uart_client[2] = open("/dev/ttyO4", O_RDWR | O_NOCTTY | O_NDELAY) < 0)
  {
    printf("Error opening UART.\r\n");
  }
  else
  {
    strcpy(ipc_struct.timestamp, getCurrentTimeStr() );
    ipc_struct.source = IPC_UART4;
    ipc_struct.destination = IPC_MAIN;
    ipc_struct.type = MSG_INFO;
    ipc_struct.comm_type = COMM_NONE;
    ipc_struct.src_pid = (pid_t)getpid();
    strcpy(ipc_struct.payload, "UART4 reference opened successfully.\r\n");
    build_ipc_msg(ipc_struct, ipc_string);
    mq_send(ipc_queue, ipc_string, strlen(ipc_string), 0);
  }

  if(uart_client[3] = open("/dev/ttyO5", O_RDWR | O_NOCTTY | O_NDELAY) < 0)
  {
    printf("Error opening UART5.\r\n");
  } 
  else
  {
    strcpy(ipc_struct.timestamp, getCurrentTimeStr() );
    ipc_struct.source = IPC_UART5;
    ipc_struct.destination = IPC_MAIN;
    ipc_struct.type = MSG_INFO;
    ipc_struct.comm_type = COMM_NONE;
    ipc_struct.src_pid = (pid_t)getpid();
    strcpy(ipc_struct.payload, "UART5 reference opened successfully.\r\n");
    build_ipc_msg(ipc_struct, ipc_string);
    mq_send(ipc_queue, ipc_string, strlen(ipc_string), 0);
  }  
  // end of UART open

  // set properties for all four UART clients
  memset(&uart_attr, 0, sizeof(uart_attr));
  tcgetattr(uart_client[0], &uart_attr);  // just get the attributes from one UART port
                                          // and set up to do a mass-edit of all client
                                          // UART ports
  // do an error check on the above

  // set terminal baud rates
//  cfsetispeed(&term_attr, B115200);
//  cfsetospeed(&term_attr, B115200);

// dropped to 9600 for now for testing purposes
  cfsetispeed(&uart_attr, B9600);
  cfsetospeed(&uart_attr, B9600);

  // set non-blocking
  uart_attr.c_cc[VMIN] = 0;
  uart_attr.c_cc[VTIME] = 100;  // set 10sec timeout (100 deciseconds per struct termios)

  // used stackoverflow post and termios man page as reference for these setup items
  uart_attr.c_cflag = (term_attr.c_cflag & ~CSIZE) | CS8;     // 8-bit chars

  uart_attr.c_iflag &= ~IGNBRK;         // disable break processing
  uart_attr.c_lflag &= ~ICANON;         // non-canonical processing
  //uart_attr.c_lflag |= ECHO;  // no need for this

  uart_attr.c_oflag = 0;                // no remapping, no delays

  uart_attr.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

  uart_attr.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
  uart_attr.c_cflag &= ~(PARENB | PARODD);      // shut off parity
  uart_attr.c_cflag |= 0;
  uart_attr.c_cflag &= ~CSTOPB;
  uart_attr.c_cflag &= ~CRTSCTS;

  // write same settings to all four UART clients
  for(int i = 0; i < MAX_UART_CLIENTS; i++)
  {
    tcsetattr(uart_client[i], TCSANOW, &uart_attr);
  }

  return 0;
}

/**
 *  @brief Handler function for UART client 0 at UART1
 */
void* comm0threadrx()
{
  // initialize comm (uart rx from client)
  // start a while loop to monitor input from uart1 (inter-board comm)
  char msg_buf[DEFAULT_BUF_SIZE]; // may need to be bigger to accomodate large image transfer chunks
  char ipc_string[DEFAULT_BUF_SIZE];

  comm_msg_t msg_struct;
  ipcmessage_t ipc_struct;
  int count = 0;

  while(bizzounce == 0)
  {
    if(uart_client[0] == 0) // uart clients not initialized, don't do anything this loop
    {
      continue;
    }
    strcpy(msg_buf, "");
    count = read(uart_client[0], msg_buf, DEFAULT_BUF_SIZE);
    if(count > 0)
    {
      hb_client_count[0] = 0;
      printf("UART1 count reset.\r\n");
      if(hb_client_err[0] == 1)
      {
        hb_client_err[0] = 0;
        
        // form message and send to main to inform that the client has connected
        strcpy(ipc_struct.timestamp, getCurrentTimeStr() );
        ipc_struct.source = IPC_UART1;
        ipc_struct.destination = IPC_MAIN;
        ipc_struct.type = MSG_INFO;
        ipc_struct.comm_type = COMM_NONE;
        ipc_struct.src_pid = (pid_t)getpid();
        strcpy(ipc_struct.payload, "Client at UART1 connected.\r\n");
        build_ipc_msg(ipc_struct, ipc_string);
        mq_send(ipc_queue, ipc_string, strlen(ipc_string), 0);
        printf("Tick UART1--.\r\n");
      }
    }

    //fscanf(uart_client, "%s", msg_buf);
    if(strlen(msg_buf) > 0)
    {
      // process from comm type to ipc message type
      //printf("Received from client: %s\n", msg_buf);
      decipher_comm_msg(msg_buf, &msg_struct);
      strcpy(ipc_struct.timestamp, msg_struct.timestamp);
      // types:
      // COMM_NONE, COMM_QUERY, COMM_DATA, COMM_INFO, COMM_CMD, COMM_ERROR, COMM_HB 
      ipc_struct.comm_type = msg_struct.type;
      strcpy(ipc_struct.payload, msg_struct.payload);
      strcpy(msg_buf, "");  // clear the buffer so we can reuse it
      
      // now build the surrounding IPC struct attributes relevant to a message coming from the TIVA
      ipc_struct.source = IPC_UART1;
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
//  printf("entered commthreadrx.\n");
}

/**
 *  @brief Handler function for UART client 1 at UART2
 */
void* comm1threadrx()
{
  // initialize comm (uart rx from client)
  // start a while loop to monitor input from uart1 (inter-board comm)
  char msg_buf[DEFAULT_BUF_SIZE]; // may need to be bigger to accomodate large image transfer chunks
  char ipc_string[DEFAULT_BUF_SIZE];

  comm_msg_t msg_struct;
  ipcmessage_t ipc_struct;
  int count = 0;

  while(bizzounce == 0)
  {
    if(uart_client[1] == 0)
    {
      continue;
    }
    strcpy(msg_buf, "");
    count = read(uart_client[1], msg_buf, DEFAULT_BUF_SIZE);
    //fscanf(uart_client, "%s", msg_buf);
    if(count > 0)
    {
      hb_client_count[1] = 0;

      if(hb_client_err[1] == 1)
      {
        hb_client_err[1] = 0;
        
        // form message and send to main to inform that the client has connected
        strcpy(ipc_struct.timestamp, getCurrentTimeStr() );
        ipc_struct.source = IPC_UART2;
        ipc_struct.destination = IPC_MAIN;
        ipc_struct.type = MSG_INFO;
        ipc_struct.comm_type = COMM_NONE;
        ipc_struct.src_pid = (pid_t)getpid();
        strcpy(ipc_struct.payload, "Client at UART2 connected.\r\n");
        build_ipc_msg(ipc_struct, ipc_string);
        mq_send(ipc_queue, ipc_string, strlen(ipc_string), 0);
      }
    }

    if(strlen(msg_buf) > 0)
    {
      // process from comm type to ipc message type
      //printf("Received from client: %s\n", msg_buf);
      decipher_comm_msg(msg_buf, &msg_struct);
      strcpy(ipc_struct.timestamp, msg_struct.timestamp);
      // types:
      // COMM_NONE, COMM_QUERY, COMM_DATA, COMM_INFO, COMM_CMD, COMM_ERROR, COMM_HB 
      ipc_struct.comm_type = msg_struct.type;
      strcpy(ipc_struct.payload, msg_struct.payload);
      strcpy(msg_buf, "");  // clear the buffer so we can reuse it
      
      // now build the surrounding IPC struct attributes relevant to a message coming from the TIVA
      ipc_struct.source = IPC_UART2;
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
//  printf("entered commthreadrx.\n");
}

/**
 *  @brief Handler function for UART client 2 at UART4
 */
void* comm2threadrx()
{
  // initialize comm (uart rx from client)
  // start a while loop to monitor input from uart1 (inter-board comm)
  char msg_buf[DEFAULT_BUF_SIZE]; // may need to be bigger to accomodate large image transfer chunks
  char ipc_string[DEFAULT_BUF_SIZE];

  comm_msg_t msg_struct;
  ipcmessage_t ipc_struct;
  int count = 0;

  while(bizzounce == 0)
  {
    if(uart_client[2] == 0)
    {
      continue;
    }
    strcpy(msg_buf, "");
    count = read(uart_client[2], msg_buf, DEFAULT_BUF_SIZE);
    //fscanf(uart_client, "%s", msg_buf);
    if(count > 0)
    {
      hb_client_count[2] = 0;

      // form message and send to main to inform that the client has connected
      strcpy(ipc_struct.timestamp, getCurrentTimeStr() );
      ipc_struct.source = IPC_UART4;
      ipc_struct.destination = IPC_MAIN;
      ipc_struct.type = MSG_INFO;
      ipc_struct.comm_type = COMM_NONE;
      ipc_struct.src_pid = (pid_t)getpid();
      strcpy(ipc_struct.payload, "Client at UART4 connected.\r\n");
      build_ipc_msg(ipc_struct, ipc_string);
      mq_send(ipc_queue, ipc_string, strlen(ipc_string), 0);
    }

    if(strlen(msg_buf) > 0)
    {
      // process from comm type to ipc message type
      //printf("Received from client: %s\n", msg_buf);
      decipher_comm_msg(msg_buf, &msg_struct);
      strcpy(ipc_struct.timestamp, msg_struct.timestamp);
      // types:
      // COMM_NONE, COMM_QUERY, COMM_DATA, COMM_INFO, COMM_CMD, COMM_ERROR, COMM_HB 
      ipc_struct.comm_type = msg_struct.type;
      strcpy(ipc_struct.payload, msg_struct.payload);
      strcpy(msg_buf, "");  // clear the buffer so we can reuse it
      
      // now build the surrounding IPC struct attributes relevant to a message coming from the TIVA
      ipc_struct.source = IPC_UART4;
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
//  printf("entered commthreadrx.\n");
}

/**
 *  @brief Handler function for UART client 3 at UART5
 */
void* comm3threadrx()
{
  // initialize comm (uart rx from client)
  // start a while loop to monitor input from uart1 (inter-board comm)
  char msg_buf[DEFAULT_BUF_SIZE]; // may need to be bigger to accomodate large image transfer chunks
  char ipc_string[DEFAULT_BUF_SIZE];

  comm_msg_t msg_struct;
  ipcmessage_t ipc_struct;
  int count = 0;

  while(bizzounce == 0)
  {
    if(uart_client[3] == 0)
    {
      continue;
    }
    strcpy(msg_buf, "");
    count = read(uart_client[3], msg_buf, DEFAULT_BUF_SIZE);
    //fscanf(uart_client, "%s", msg_buf);
    if(count > 0)
    {
      hb_client_count[3] = 0;
      // form message and send to main to inform that the client has connected
      strcpy(ipc_struct.timestamp, getCurrentTimeStr() );
      ipc_struct.source = IPC_UART5;
      ipc_struct.destination = IPC_MAIN;
      ipc_struct.type = MSG_INFO;
      ipc_struct.comm_type = COMM_NONE;
      ipc_struct.src_pid = (pid_t)getpid();
      strcpy(ipc_struct.payload, "Client at UART5 connected.\r\n");
      build_ipc_msg(ipc_struct, ipc_string);
      mq_send(ipc_queue, ipc_string, strlen(ipc_string), 0);      
    }

    if(strlen(msg_buf) > 0)
    {
      // process from comm type to ipc message type
      //printf("Received from client: %s\n", msg_buf);
      decipher_comm_msg(msg_buf, &msg_struct);
      strcpy(ipc_struct.timestamp, msg_struct.timestamp);
      // types:
      // COMM_NONE, COMM_QUERY, COMM_DATA, COMM_INFO, COMM_CMD, COMM_ERROR, COMM_HB 
      ipc_struct.comm_type = msg_struct.type;
      strcpy(ipc_struct.payload, msg_struct.payload);
      strcpy(msg_buf, "");  // clear the buffer so we can reuse it
      
      // now build the surrounding IPC struct attributes relevant to a message coming from the TIVA
      ipc_struct.source = IPC_UART5;
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
//  printf("entered commthreadrx.\n");
}

/**
 *  @brief Decipher comm string into comm_msg_t message type
 *  @param comm_msg input string
 *  @param msg_struct output comm msg struct
 */
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

/**
 *  @brief Build comm string into comm_msg_t message type
 *  @param msg_struct input comm msg struct
 *  @param comm_msg output string for transmit
 */
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

/**
 *  @brief Decipher comm payload into useful data or commands.
 *  @param data_t data payload
 *  @param payload input payload string to decipher
 * Function is not used - was planned to be but did not get far enough
 */
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

/**
 *  @brief Build comm payload into useful data or commands.
 *  @param data_t data payload
 *  @param payload input payload string to build
 * Function is not used - was planned to be but did not get far enough
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

/**
 *  @brief Transmit string over selected UART port
 *  @param uart_t input UART handle
 *  @param buffer string to transmit
 *  @return return status
 */
int uart_write(uart_t uart, char* buffer)
{
  pthread_mutex_lock(&uart_mutex);
  int n = write(uart, buffer, strlen(buffer) );
  pthread_mutex_unlock(&uart_mutex);

  if(n < 0 || uart == 0)
  {
    return -1;
  }

  return n;
}

/**
 *  @brief Read string from selected UART port of size count
 *  @param uart_t input UART handle
 *  @param buffer string to store output in
 *  @param count number of bytes to read
 *  @return return status
 */
int uart_read(uart_t uart, char* buffer, int count)
{
  int n;

  if(buffer == NULL || count < 1 || uart == 0)
  {
    return -1;
  }
  
  pthread_mutex_lock(&uart_mutex);
  n = read(uart, buffer, count );
  pthread_mutex_unlock(&uart_mutex);

  if(n < 0)
  {
    return -1;
  }

  return n;
}
