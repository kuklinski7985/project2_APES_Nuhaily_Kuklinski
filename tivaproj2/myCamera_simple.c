/**
* @file camera.c
* @brief operation functions for the Adafruit TTL serial JPEG camera, VC0706
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

#include "myCamera_simple.h"
/*these definitions are from the perspective of the camera
 * They are signals that are sent from the host to the camera
 */

//commands for the camera
uint8_t stop_frame[] = {PROTO_SIGN_RECV, SN_NUM, CMD_FBUF_CTRL, 0x01, NADA};

uint8_t data_length_current_frame[] = {PROTO_SIGN_RECV, SN_NUM, CMD_GET_FBUF_LEN, 0X01, NADA};


/*return data should be formatted like this:
 * 76 00 32 00 00 ffd8 ...imagine data... ffd9 76 00 32 00 00
 * JPEG bookended with 0xFFD8.....0xFFD9
 *
 * data_length: size of chunk of data to read at one time, divisable by 8
 * delay: recommended to be 0x000A
 */

uint8_t pic_data_return[] = {PROTO_SIGN_RECV, SN_NUM, CMD_READ_FBUF, 0x0c, FBUF_TYPE_CURRENT,
                             RD_FBUF_CTRL_MCU, RD_FBUF_START_ADDR_1, RD_FBUF_START_ADDR_2, RD_FBUF_START_ADDR_AH,
                             RD_FBUF_START_ADDR_LH, DATA_GRAB_LEN, DELAY_H, DELAY_L};

//return camera to video mode, continuously taking frames
uint8_t stop_take_pic[] = {PROTO_SIGN_RECV, SN_NUM, CMD_FBUF_CTRL, 0x01, FBUF_CTRL_STEP_FRM};

uint8_t set_image_size[] = {PROTO_SIGN_RECV, SN_NUM, CMD_IMAGE_SIZE, 0x05, 0x04, 0x19};

char pic_file[10000] = {0};
uint8_t pic_size = 0;
uint32_t data_length = 0;               //used to set length of data in the pic_data_return array, end address


void UARTCamera_sendCommand(uint8_t *cmd_array, uint8_t array_len)
{
    uint8_t i;
    for(i =0; i < array_len;i++)
    {
        ROM_UARTCharPut(UART5_BASE, cmd_array[i]);
        //UARTprintf("[send]: 0x%02x\n");
    }
    //UARTprintf("waiting............\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void setImageSize(uint8_t picsize)
{
    uint8_t my_args[] = {picsize};
    UARTCamera_sendCommand(set_image_size, SET_IMAGE_SIZE_LENGTH);
    UARTCamera_sendCommand(my_args, 0x01);

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

void clear_recv_buffer(void)
{
    //ADD MUTEX HERE, SAME ONE THAT SHOULD BE ASSOCIATED WITH INTERRUPT
    uint8_t i;
    for(i = 0; i < CAMERA_RECV_BUFF_LENGTH; i++)
    {
        camera_data_recv[i] = 0;
    }
}

uint8_t takePicture(void)
{
    /*
     * 3. start saving to file with FFD8 and end with FFD9
     * 4. stop taking pic, set camera back to video capture
     */
    uint32_t file_length;
    clear_recv_buffer();
    //stopping frame refresh of camera
    UARTCamera_sendCommand(stop_frame,STOP_FRAME_LENGTH);
    while(!camera_handler_exit_flag)
    {
        //wait until all bytes of response are received from interrupt
    }
    //getting pic file size
    clear_recv_buffer();
    UARTCamera_sendCommand(data_length_current_frame, DATA_LENGTH_LENGTH);
    while(!camera_handler_exit_flag)
    {
        //wait until all bytes of response are received from interrupt
    }

    //Retrieving total file length from array
    file_length = (camera_data_recv[5]<<24) + (camera_data_recv[6]<<16)+
            (camera_data_recv[7]<<8) + camera_data_recv[8];
    UARTprintf("Pic File Size %f\n",file_length);

    clear_recv_buffer();

    return 1;
}
