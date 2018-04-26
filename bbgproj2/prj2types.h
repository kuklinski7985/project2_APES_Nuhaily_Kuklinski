#ifndef PRJ2_DEF_H_
#define PRJ2_DEF_H_

#define DEFAULT_BUF_SIZE   256

/*types of ipc messages that are possible*/
typedef enum{
  MSG_NONE, MSG_QUERY, MSG_DATA, MSG_INFO, MSG_TERMINATE, MSG_ERROR, MSG_HB
} message_t;

/*locations messages can be sent to and received from*/
typedef enum{
  IPC_NONE, IPC_LOG, IPC_MAIN, IPC_SOCKET, IPC_USER, IPC_HB, IPC_TEMP, IPC_LIGHT
} location_t;

// Server-client message types
typedef enum {
  COMM_NONE, COMM_QUERY, /*COMM_RESPONSE, */COMM_DATA, COMM_INFO, COMM_CMD, COMM_ERROR, COMM_HB 
} comm_t;

typedef enum {
    DATA_NONE, DATA_RFID, DATA_IMG
} sensor_t;

typedef struct data {
    sensor_t sensor_type;
    int sensorid;
    uint32_t data;
} data_t;

// right now it doesn't know what comm_t is because I can't include comm.h - the include becomes circular since comm.h needs ipc_messq.h
// how do I handle this? I want to keep a data element of type comm_t in this struct
/*struct to define messages passed around to all parts of the system*/
typedef struct ipcmessage {
  char timestamp[10];
  message_t type;                   //message identifier
  comm_t comm_type;                 // inter-board message type
  location_t source;                // where message originates from
  pid_t src_pid;                    // pid of process creating the message
  location_t destination;           // final destination for message
  char payload[DEFAULT_BUF_SIZE];   // message to transmit
} ipcmessage_t;

#endif