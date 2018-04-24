/**
* @file RFID_SM130.h
* @brief prototype functions for the SM130 rfid reader
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include <stdio.h>
#include <stdint.h>
#define RECV_BUFF_LENGTH    32

#ifndef RFID_SM130_h_
#define RFID_SM130_h_

extern uint8_t rfid_handler_exit_flag;

char rfid_data_recv[RECV_BUFF_LENGTH];

void RFID_reset();

void RFID_seek();

void RFID_antPWR();

/*if using this function, you will not know if the changes have taken affect
 * until you change the UART baud rate to the new overall rate
 * and then reset the system
 */
void RFID_setbaud(uint32_t newBaud);

#endif /*_RFID_SM130_h_*/
