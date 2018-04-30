/**
* @file RFID_SM130.c
* @brief operation fxunctions for the RFID Sm130 RFID tag reader
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include "RFID_SM130.h"
//#include "main.h"

ipcmessage_t RFID_IPC_mess;
extern QueueHandle_t ipc_msg_queue;
char snd_str[256];

#define HEADER          0xff;    //always 0xff
#define RFIDRESERVE     0x00;   //always 0x00

void RFID_reset()
{
    rfid_handler_exit_flag = 0;
    char sn_num[64] = {0};

    uint8_t i =0;
    for(i=0; i<RFID_RECV_BUFF_LENGTH; i++)
    {
        rfid_data_recv[i] = 0;
    }
    uint8_t elements = 5;
    char send_comm_buff[6] = {0};
    uint8_t rfidlength = 0x01;
    uint8_t rfidcommand = 0x80;

    uint8_t rfidcsum = 0x00;
    rfidcsum = rfidlength + rfidcommand;
    send_comm_buff[0] = HEADER;
    send_comm_buff[1] = RFIDRESERVE;
    send_comm_buff[2] = rfidlength;
    send_comm_buff[3] = rfidcommand;
    send_comm_buff[4] = rfidcsum;
    /*for(i=0; i<elements; i++)
    {
        UARTprintf("sent[%d]: %02x\n",i,send_comm_buff[i]);
    }*/
    UARTRFID_send(send_comm_buff, elements);

    //UARTprintf("Command sent to module, waiting for response\n");
    while(rfid_handler_exit_flag == 0)
    {
    }

    getTimeStamp(&RFID_IPC_mess);
    RFID_IPC_mess.type = MSG_INFO;
    RFID_IPC_mess.source = TASK_RFID;
    RFID_IPC_mess.destination = TASK_LOGGING;
    sprintf(sn_num, "RFID SN# %c%c%c%c%c%c%c%c",rfid_data_recv[4],
                    rfid_data_recv[5],rfid_data_recv[6],rfid_data_recv[7],rfid_data_recv[8],
                    rfid_data_recv[9],rfid_data_recv[10],rfid_data_recv[11]);
    /*strcpy(RFID_IPC_mess.payload, sn_num);
    build_ipc_msg(RFID_IPC_mess, snd_str);
    xQueueSend(ipc_msg_queue, snd_str, portMAX_DELAY);*/

    /*
     * For the sake of a proper demo, I am printing out the received time stamp and ID string to the terminal via UARTprintf,
     * and not through the IPC messaging system.  For some reason, the program cannot find the queue, the
     *  debugger giving an error of "Cannot load from Primitive location".  Also suspect string and array size.
     */

    UARTprintf("%s | %s\n", RFID_IPC_mess.timestamp, sn_num);
    /*UARTprintf("Reset Complete. RFID Version: ");
    for(i=4; i<11; i++)
    {
        UARTprintf("%c", rfid_data_recv[i]);
    }
    UARTprintf("\n");*/
}


void RFID_seek()
{
    rfid_handler_exit_flag = 0;


    char sn_tag_found[64];
    //char snd_str[256];

    //ipcmessage_t RFID_IPC_mess;
    RFID_IPC_mess.type = MSG_INFO;
    RFID_IPC_mess.source = TASK_RFID;
    RFID_IPC_mess.destination = TASK_LOGGING;

    uint8_t i =0;

    for(i=0; i<RFID_RECV_BUFF_LENGTH; i++)
    {
        rfid_data_recv[i] = 0;
    }

    uint8_t elements = 5;
    char send_comm_buff[6] = {0};
    uint8_t rfidlength = 0x01;
    uint8_t rfidcommand = 0x82;
    //uint8_t rfiddata = 0x00;

    uint8_t rfidcsum = 0x00;
    rfidcsum = rfidlength + rfidcommand;
    send_comm_buff[0] = HEADER;
    send_comm_buff[1] = RFIDRESERVE;
    send_comm_buff[2] = rfidlength;
    send_comm_buff[3] = rfidcommand;
    send_comm_buff[4] = rfidcsum;

    UARTRFID_send(send_comm_buff, elements);


    while(rfid_handler_exit_flag == 0)
    {
    }
    rfid_handler_exit_flag = 0;
    /*if(rfid_data_recv[4] == 0x4c)
    {
        getTimeStamp(&RFID_IPC_mess);
        strcpy(RFID_IPC_mess.payload, "Seeking Mode for RFID");
        build_ipc_msg(RFID_IPC_mess, snd_str);
        xQueueSend(ipc_msg_queue, snd_str, portMAX_DELAY);
        //UARTprintf("Seeking RFID tag...\n");
    }
    if(rfid_data_recv[4]==0x55)
    {
        RFID_IPC_mess.type = MSG_ERROR;
        strcpy(RFID_IPC_mess.payload, "RFID detection field off");
        build_ipc_msg(RFID_IPC_mess, snd_str);
        xQueueSend(ipc_msg_queue, snd_str, portMAX_DELAY);
        //UARTprintf("Seeking with RF field OFF!!\n");
    }*/

    for(i=0; i<RFID_RECV_BUFF_LENGTH; i++)
    {
        rfid_data_recv[i] = 0;
    }

    while(rfid_handler_exit_flag == 0)
    {
    }

    switch(rfid_data_recv[4]){
    case 0x01:
        sprintf(sn_tag_found, "TAG SN# %02x %02x %02x %02x %02x | Ultralight",rfid_data_recv[4],
                rfid_data_recv[5],rfid_data_recv[6],rfid_data_recv[7],rfid_data_recv[8]);
        getTimeStamp(&RFID_IPC_mess);
        //UARTprintf("Mifare Ultralight\n");

        break;
    case 0x02:
        sprintf(sn_tag_found, "TAG SN# %02x %02x %02x %02x %02x | Standard 1K",rfid_data_recv[4],
                        rfid_data_recv[5],rfid_data_recv[6],rfid_data_recv[7],rfid_data_recv[8]);
        getTimeStamp(&RFID_IPC_mess);
        //UARTprintf("Mifare Standard 1K\n");
        break;
    case 0x03:
        sprintf(sn_tag_found, "TAG SN# %02x %02x %02x %02x %02x | Classic 4K",rfid_data_recv[4],
                        rfid_data_recv[5],rfid_data_recv[6],rfid_data_recv[7],rfid_data_recv[8]);
        getTimeStamp(&RFID_IPC_mess);
        //UARTprintf("Mifare Classic 4K\n");
        break;
    case 0xFF:
        UARTprintf("Unknown Tag Type\n");
        break;
    }
    /*strcpy(RFID_IPC_mess.payload, sn_tag_found);
    build_ipc_msg(RFID_IPC_mess, snd_str);
    xQueueSend(*((QueueHandle_t*)&ipc_msg_queue), snd_str, portMAX_DELAY);*/

    /*
     * For the sake of a proper demo, I am printing out the received time stamp and ID string to the terminal via UARTprintf,
     * and not through the IPC messaging system.  For some reason, the program cannot find the queue, the
     *  debugger giving an error of "Cannot load from Primitive location".  Also suspect string and array size.
     */
    UARTprintf("%s | %s\n", RFID_IPC_mess.timestamp, sn_tag_found);
}

void RFID_antPWR()
{

}

void RFID_setbaud(uint32_t newBaud)
{
    rfid_handler_exit_flag = 0;

    uint8_t i =0;
    for(i=0; i<RFID_RECV_BUFF_LENGTH; i++)
    {
        rfid_data_recv[i] = 0;
    }
    uint8_t elements = 6;
    char send_comm_buff[7] = {0};
    uint8_t rfidlength = 0x02;
    uint8_t rfidcommand = 0x94;
    //data is used to select the baud rate
    /*0x00 - 9600bps
     *0x01 - 19200bps
     *0x02 - 38400bps
     *0x03 - 57600bps
     *0x04 - 115200bps
     */
    uint8_t rfiddata;
    switch(newBaud){
    case 9600:
        rfiddata = 0x00;
        break;
    case 19200:
        rfiddata = 0x01;
        break;
    case 38400:
        rfiddata = 0x02;
        break;
    case 57600:
        rfiddata = 0x03;
        break;
    case 115200:
        rfiddata = 0x04;
        break;
    default:
        UARTprintf("Baud Not recognized, Set to 115200\n");
        rfiddata = 0x04;
        break;
    }


    UARTprintf("just after swtich %d\n",rfiddata);
    uint8_t rfidcsum = 0x00;
    rfidcsum = rfidlength + rfidcommand + rfiddata;
    send_comm_buff[0] = HEADER;
    send_comm_buff[1] = RFIDRESERVE;
    send_comm_buff[2] = rfidlength;
    send_comm_buff[3] = rfidcommand;
    send_comm_buff[4] = rfiddata;
    send_comm_buff[5] = rfidcsum;

    UARTRFID_send(send_comm_buff, elements);

    for(i=0; i<elements; i++)
    {
        UARTprintf("sent[%d]: %02x\n",i,send_comm_buff[i]);
    }

    UARTprintf("Baud change in Progress...\n");
    while(rfid_handler_exit_flag == 0)
    {
    }
    if(rfid_data_recv[4]==0x4e)
    {
        UARTprintf("Baud Rate Change Failed");
    }
    if(rfid_data_recv[4]==0x4C)
    {
        UARTprintf("Baud Rate Success.  If reading this unchanged.\n");
    }

}

void UARTRFID_send(char * RFID_buffer, uint32_t byteCount)
{
    int i =0;
    for(i =0; i<byteCount;i++)
    {
        ROM_UARTCharPut(UART6_BASE, RFID_buffer[i]);
    }
}
