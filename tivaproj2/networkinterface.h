/**
* @file networkinterface.h
* @brief MAC TCP/IP interaction fxn prototypes
* @author Andrew Kuklinski and Adam Nuhaily
* @date 04/28/2018
**/

/*Code structure obtained from FreeRTOS website
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/Embedded_Ethernet_Porting.html#xNetworkInterfaceInitialise
 * and the TivaWare Peripheral Driver Library, Chapter 10
 * http://www.ti.com/lit/ug/spmu298d/spmu298d.pdf
 * Modified by Andrew Kuklinski
 */

//#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"
#include "inc/hw_emac.h"
#include "inc/hw_flash.h"
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
#include "driverlib/flash.h"
#include "utils/uartstdio.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"
#include "task.h"
#include "timers.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "emac.h"
#include "NetworkBufferManagement.h"


#ifndef networkinterface_h_
#define networkinterface_h_

//MAC hardware needs 3 receive descriptors to operate.
#define NUM_TX_DESCRIPTORS 3
#define NUM_RX_DESCRIPTORS 3

TaskHandle_t socketTask;


extern uint32_t f_sysclk;

tEMACDMADescriptor g_psRxDescriptor[NUM_TX_DESCRIPTORS];
tEMACDMADescriptor g_psTxDescriptor[NUM_RX_DESCRIPTORS];
uint32_t g_ui32RxDescIndex;
uint32_t g_ui32TxDescIndex;

#define RX_BUFFER_SIZE 1536
uint8_t g_ppui8RxBuffer[NUM_RX_DESCRIPTORS][RX_BUFFER_SIZE];


/*initializes the MAC and PHY layers for interface to the TCP/IP layer*/
BaseType_t xNetworkInterfaceInitialise( void );

void eth_MAC_init( void );

/*transmit a packet from the supplied buffer, called directly from the application
 * pui8Buf points to ethernet frame to send, i32BufLen number of bytes in the frame
 */
//static int32_t PacketTransmit(uint8_t *pui8Buf, int32_t i32BufLen);

BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t * const pxDescriptor, BaseType_t xReleaseAfterSend );

//void vNetworkInterfaceAllocateRAMToBuffers(NetworkBufferDescriptor_t xDescriptors[ ipconfigNUM_NETWORK_BUFFERS ] );
//void vNetworkInterfaceAllocateRAMToBuffers(NetworkBufferDescriptor_t xDescriptors[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] );


/*Initializes the transmit and receive DMA descriptors*/
void InitDescriptors(uint32_t ui32Base);

/*Read a packet from the DMA receive buffer and return the number of btyes read*/
int32_t ProcessReceivedPacket(void);

//interrupt handler for the ethernet interrupt
void EthernetIntHandler(void);



void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent );

void vSocketTask(void *pvParameters);


#endif /*_networkinterface_h_*/
