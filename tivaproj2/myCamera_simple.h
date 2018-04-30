/**
* @file camera.h
* @brief prototype functions for the adafruit TTL serial JPEG camera with NTSC video
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

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

#ifndef myCamera_simple_h_
#define myCamera_simple_h_

/*control bytes for camera and array definitions for control*/
#define PROTO_SIGN_RECV             0X56
#define PROTO_SIGN_SEND             0X76
#define SN_NUM                      0X00
#define NADA                        0x00
#define FBUF_TYPE_CURRENT           0X00
#define FBUF_TYPE_NEXT              0x01
#define RD_FBUF_CTRL_MCU            0x0c
#define RD_FBUF_CTRL_DMA            0x01
#define RD_FBUF_START_ADDR_1        0x00
#define RD_FBUF_START_ADDR_2        0x00
#define RD_FBUF_START_ADDR_AH       0x00
#define RD_FBUF_START_ADDR_LH       0x00
#define FBUF_CTRL_RESUME_FRM        0X02
#define FBUF_CTRL_STEP_FRM          0X03

#define CMD_GET_VERSION             0x11
#define CMD_SYS_RESET               0X26
#define CMD_FBUF_CTRL               0x36
#define CMD_GET_FBUF_LEN            0X34
#define CMD_READ_FBUF               0X32
#define CMD_IMAGE_SIZE              0X31



//use these values to save the responses received from the camera
#define CAMERA_RECV_BUFF_LENGTH     100
char camera_data_recv[CAMERA_RECV_BUFF_LENGTH];
extern uint8_t camera_handler_exit_flag;

//defines for length of each array, used for send_command function
#define GET_VERSION_LENGTH          4
#define SYS_RESET_LENGTH            4
#define STOP_FRAME_LENGTH           5
#define DATA_LENGTH_LENGTH          5
#define PIC_DATA_RETURN_LENGTH      13
#define STOP_TAKE_PIC_LENGTH        5
#define SET_IMAGE_SIZE_LENGTH       6

//defines for pic size selection

#define SIZE_640x480        0x00
#define SIZE_320x240        0x11
#define SIZE_160x120        0x22



#define DELAY_H                     0x00
#define DELAY_L                     0x0A
#define DATA_GRAB_LEN               0Xff
//uint8_t delay_H = 0x00;                     //used to set delay in pic_data_return array, 2 bytes, delay*0.01ms
//uint8_t delay_L = 0x0A;

//sends properly formatted commands to the camera
void UARTCamera_sendCommand(uint8_t *cmd_array, uint8_t array_len);

//obtain image size
void setImageSize(uint8_t picsize);

//buffer flush
void clear_recv_buffer(void);

//sends proper series of commands to take a picture
uint8_t takePicture(void);

#endif /*_myCamera_simple_h_*/


