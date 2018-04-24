/**
* @file RFID_SM130.c
* @brief operation fxunctions for the RFID Sm130 RFID tag reader
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include "RFID_SM130.h"
#include "main.h"


#define HEADER          0xff;    //always 0xff
#define RFIDRESERVE     0x00;   //always 0x00

void RFID_reset()
{
    rfid_handler_exit_flag = 0;

    uint8_t i =0;
    for(i=0; i<RECV_BUFF_LENGTH; i++)
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
    for(i=0; i<elements; i++)
    {
        UARTprintf("sent[%d]: %02x\n",i,send_comm_buff[i]);
    }
    UARTRFID_send(send_comm_buff, elements);

    UARTprintf("Command sent to module, waiting for response\n");
    while(rfid_handler_exit_flag == 0)
    {
    }
    UARTprintf("Reset Complete. RFID Version: ");
    for(i=4; i<11; i++)
    {

        UARTprintf("%c", rfid_data_recv[i]);
    }
    UARTprintf("\n");
}


void RFID_seek()
{
    rfid_handler_exit_flag = 0;

    uint8_t i =0;

    for(i=0; i<RECV_BUFF_LENGTH; i++)
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
    if(rfid_data_recv[4] == 0x4c)
    {
        UARTprintf("Seeking RFID tag...\n");
    }
    if(rfid_data_recv[4]==0x55)
    {
        UARTprintf("Seeking with RF field OFF!!\n");
    }

    for(i=0; i<RECV_BUFF_LENGTH; i++)
    {
        rfid_data_recv[i] = 0;
    }

    while(rfid_handler_exit_flag == 0)
    {
    }
    UARTprintf("Tag Detected. SN#: ");
    for(i=4; i<(rfid_data_recv[2]+3); i++)
    {
        UARTprintf("%02x ", rfid_data_recv[i]);
    }
    UARTprintf(" Tag Type: ");
    switch(rfid_data_recv[4]){
    case 0x01:
        UARTprintf("Mifare Ultralight\n");
        break;
    case 0x02:
        UARTprintf("Mifare Standard 1K\n");
        break;
    case 0x03:
        UARTprintf("Mifare Classic 4K\n");
        break;
    case 0xFF:
        UARTprintf("Unknown Tag Type\n");
        break;
    }

}

void RFID_antPWR()
{

}

void RFID_setbaud(uint32_t newBaud)
{
    rfid_handler_exit_flag = 0;

    uint8_t i =0;
    for(i=0; i<RECV_BUFF_LENGTH; i++)
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


