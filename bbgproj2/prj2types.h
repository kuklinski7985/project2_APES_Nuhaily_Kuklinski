#ifndef PRJ2_DEF_H_
#define PRJ2_DEF_H_

#define DEFAULT_BUF_SIZE   256
#define MAX_UART_CLIENTS   4

// types of ipc messages that are possible
typedef enum{
  MSG_NONE, MSG_QUERY, MSG_DATA, MSG_INFO, MSG_TERMINATE, MSG_ERROR, MSG_HB
} message_t;

// locations messages can be sent to and received from
typedef enum{
  IPC_NONE, IPC_LOG, IPC_MAIN, IPC_SOCKET, IPC_USER, IPC_HB, IPC_UART1, 
  IPC_UART2, IPC_UART3, IPC_UART4, IPC_LOOPBACK
} location_t;

// Server-client message types
typedef enum {
  COMM_NONE, COMM_QUERY, /*COMM_RESPONSE, */COMM_DATA, COMM_INFO, COMM_CMD, 
  COMM_ERROR, COMM_HB 
} comm_t;

typedef enum {
    DATA_NONE, DATA_RFID, DATA_IMG
} sensor_t;

typedef struct data {
    sensor_t sensor_type;
    int sensorid;
    uint32_t data;
} data_t;

// struct to define messages passed between boards (server - client)
typedef struct comm_msg {
  char timestamp[10];
  comm_t type;                          // message identifier
  char payload[DEFAULT_BUF_SIZE];    // message to transmit
} comm_msg_t;

// struct to define messages passed around to all parts of the system
typedef struct ipcmessage {
  char timestamp[10];
  message_t type;                   // message identifier
  comm_t comm_type;                 // inter-board message type
  location_t source;                // where message originates from
  pid_t src_pid;                    // pid of process creating the message
  location_t destination;           // final destination for message
  char payload[DEFAULT_BUF_SIZE];   // message to transmit
} ipcmessage_t;

#endif