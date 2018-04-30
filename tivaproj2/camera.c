/**
* @file camera.c
* @brief operation functions for the Adafruit TTL serial JPEG camera, VC0706
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include "camera.h"


void UARTCamera_sendCommand(uint8_t cmd, uint8_t send_args[], uint8_t arg_num)
{
    uint8_t i;
    ROM_UARTCharPut(UART4_BASE, SENDPROTOBYTE);
    ROM_UARTCharPut(UART4_BASE, CAMERASNNUM);
    ROM_UARTCharPut(UART4_BASE, cmd);

    for(i =0; i < arg_num;i++)
    {
        ROM_UARTCharPut(UART4_BASE, send_args[i]);
        //UARTCharPutNonBlocking(UART4_BASE, send_args[i]);
    }
    UARTprintf("sent to camera[0]: %02x\n",SENDPROTOBYTE);
    UARTprintf("sent to camera[1]: %02x\n",CAMERASNNUM);
    UARTprintf("sent to camera[2]: %02x\n",cmd);
    uint8_t num = 3;
    for(i=0; i< arg_num; i++ )
    {
        UARTprintf("sent to camera[%d]: %02x\n",num,send_args[i]);
        num++;
    }

}

void getVersion(void)
{
    UARTprintf("Getting version info........\n");
    uint8_t send_args[] = {0x00};
    UARTCamera_sendCommand(0x11, send_args, 1);

}

void systemRest(void)
{
    UARTprintf("Sending Reset.......\n");
    uint8_t send_args[] = {0x00};
    UARTCamera_sendCommand(0x26, send_args, 1);

}

void setImageSize(uint8_t picsize)
{
    uint8_t send_args[] = {0x05, 0x04, 0x01, 0x00, 0x19, picsize};
    UARTCamera_sendCommand(0x31,send_args,6);
    switch (picsize){
    case SIZE_160x120:
        UARTprintf("Pic Size: 160x120\n");
        break;
    case SIZE_320x240:
        UARTprintf("Pic Size: 320x240\n");
        break;
    case SIZE_640x480:
        UARTprintf("Pic Size: 640x480\n");
        break;
    default:
        UARTprintf("Unknown Size...\n");
        break;
    }
}

uint8_t takePicture()
{

    return 1;
}

uint8_t cameraFrameBuffCtrl(uint8_t cmd)
{
    uint8_t args[] = {0x1, cmd};
    sendCommand(cmd,args,);
    return runCommand(VC0706_FBUF_CTRL, args, sizeof(args), 5);
}

uint8_t runCommand(uint8_t cmd, uint8_t *args, uint8_t argn, uint8_t resplen)
{

  sendCommand(cmd, args, argn);
  if (readResponse(resplen, 200) != resplen)
    return false;
  if (! verifyResponse(cmd))
    return false;
  return true;
}

/*boolean Adafruit_VC0706::takePicture() {
  frameptr = 0;
  return cameraFrameBuffCtrl(VC0706_STOPCURRENTFRAME);
}*/

/*
 * protocal sign: receive 0x56, send 0x76
 * data length: does not include proto sign, sn command or legnth.  ONLY DATA!

 *
 * Receive protocal format (from host to camera)
 * Protocol sign (0x56) | sn # (1) | command (1) | data length (1) | data(0-16)
 *
 * Send Protocol Format (from camera to host)
 * Protocol sign (0x76) | sn # (1) | command (1) | status (1) | data lengths (1) | data(0-16)
 *
 *
 */
