/**
* @file comm.h
* @brief fxn definition for queue creation and use
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#define DEFAULT_BUFFER_SIZE     256

// Server-client message types
typedef enum {
  COMM_NONE, COMM_QUERY, /*COMM_RESPONSE, */COMM_DATA, COMM_INFO, COMM_CMD, COMM_ERROR, COMM_HB 
} comm_t;

typedef enum{
    DATA_NONE, DATA_RFID, DATA_IMG
} sensor_t;

typedef struct data {
    sensor_t sensor_type;
    int sensorid;
    uint32_t data;
} data_t;

// we don't really need a location type do we? if the server is sending a message it's obviously going to the client
// if the client receives a message it was obiviously from the server
// I guess if we have multiple clients we could have the server know which client it was by identifying what port it received it from

// struct to define messages passed around to all parts of the system
typedef struct comm_msg {
  char timestamp[10];
  comm_t type;                          // message identifier
  char payload[DEFAULT_BUFFER_SIZE];    // message to transmit
} comm_msg_t;

void decipher_comm_msg(char* comm_msg, comm_msg_t* msg_struct);
void build_comm_msg(comm_msg_t msg_struct, char* comm_msg);

void decipher_comm_data(data_t comm_data, char* payload);
void build_comm_data(char* payload, data_t* comm_data);

// message format (if it contains sensor data): <type>\n<timestamp>\n<data_t>|<payload>
