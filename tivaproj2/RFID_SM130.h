/**
* @file RFID_SM130.h
* @brief prototype functions for the SM130 rfid reader
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include <stdio.h>
#include <stdint.h>

#ifndef RFID_SM130_h_
#define RFID_SM130_h_

char rfid_data_recv[32];

void RFID_reset();

void RFID_firmware();

void RFID_seek();

void RFID_antPWR();

void RFID_setbaud();



#endif /*_RFID_SM130_h_*/
