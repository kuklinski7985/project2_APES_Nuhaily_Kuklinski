/**
* @file camera.h
* @brief prototype functions for the adafruit TTL serial JPEG camera with NTSC video
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

/*
 * Code from Adafruit library referenced and adapted for use in project2
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"


// FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"
#include "task.h"
#include "timers.h"
#include "FreeRTOS_sockets.h"

// Project 2 includes
#include "ipc_message.h"

#define SENDPROTOBYTE       0x56
#define RECVPROTOBYTE       0x76

#define SIZE_640x480        0x00
#define SIZE_320x240        0x11
#define SIZE_160x120        0x22

#define CAMERA_RECV_BUFF_LENGTH    100
char camera_data_recv[CAMERA_RECV_BUFF_LENGTH];


#ifndef camera_h
#define camera_h

//global to hold the SN number, set when task first starts
//uint8_t cameraSNnum = 0x00;

#define CAMERASNNUM     0X00


//sets type of communication, port
//void setPort();

//send command to reset the system
//void systemReset();

//send the command necessary for camera operations
void UARTCamera_sendCommand(uint8_t cmd, uint8_t send_args[], uint8_t arg_num);

//flushes buffer, sends the command, reads the response, and verifies the response
//not sure if this is needed
//boolean runCommand(uint8_t cmd, uint8_t *args, uint8_t argen, uint8_t resplen, boolean flushflag);

//gets the current software version of the camera, command byte 0x11
void getVersion(void);


void systemRest(void);

//sets image size to one of three options
void setImageSize(uint8_t picsize);


//takes picture, stops the current frame refresh so buffer can be read out
uint8_t takePicture(void);

//not sure is this is needed, referenced in takePicture()
uint8_t cameraFrameBuffCtrl(uint8_t command);

//asks the camera how big the picture buffer is, returns the size in bytes
uint32_t getFrameLength(void);

//reads camera buffer pic inforamtion into a byte array
//runs command to read buff
uint8_t readPicture(uint8_t n);





#endif /*_camera_h_*/

/*
 * protocal sign: receive 0x56, send 0x76
 * data length: does not include proto sign, sn command or legnth.  ONLY DATA!

 *
 * Receive protocal format (into camera from Host)
 * Protocol sign (1) | sn # (1) | command (1) | data length (1) | data(0-16)
 *
 * Send Protocol Format (from camera to host)
 * Protocol sign (1) | sn # (1) | command (1) | status (1) | data lengths (1) | data(0-16)
 *
 *
 */












