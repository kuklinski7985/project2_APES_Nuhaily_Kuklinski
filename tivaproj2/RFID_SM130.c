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
    rfid_handlder_exit_flag = 0;
    //command 0x80
    //no data
    //example command FF 00 01 80 81
    //host sending
    //header 0xff | reserved 0x00 | length (command and data) | command | data | csum (add all)
    //module receiving
    //header 0xff | reserver 0x00 | length (payload data) | command | data (nbytes) | csum

    uint8_t i =0;//,j = 0;
    uint8_t elements = 5;
    char send_comm_buff[6] = {0};
    uint8_t rfidlength = 0x01;
    uint8_t rfidcommand = 0x80;

    //this might need to change to an array if data of more than 1 byte is necessary
    uint8_t rfiddata = 0x00;

    uint8_t rfidcsum = 0x00;
    rfidcsum = rfidlength + rfidcommand + rfiddata;
    send_comm_buff[0] = HEADER;
    send_comm_buff[1] = RFIDRESERVE;
    send_comm_buff[2] = rfidlength;
    send_comm_buff[3] = rfidcommand;
    //send_comm_buff[4] = rfiddata;
    send_comm_buff[4] = rfidcsum;
    for(i=0; i<elements; i++)
    {
        UARTprintf("sent[%d]: %02x\n",i,send_comm_buff[i]);
    }
    UARTRFID_send(send_comm_buff, 5);
    UARTprintf("Command sent to module, waiting for response\n");
    while(rfid_handlder_exit_flag == 0)
    {
    }
}

void RFID_firmware()
{
    /*uint8_t i = 0;
    uint8_t elements = 5;
    uint8_t send_comm_buff[6] = {0};
    uint8_t rfidlength = 0x01;
    uint8_t rfidcommand = 0x81;*/


}

void RFID_seek()
{

}

void RFID_antPWR()
{

}

void RFID_setbaud()
{

}


