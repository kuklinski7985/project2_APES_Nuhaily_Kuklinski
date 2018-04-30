/**
* @file networkinterface.c
* @brief translation functions for MAC - TCP/IP communication
* @author Andrew Kuklinski and Adam Nuhaily
* @date 03/11/2018
**/

/*Code structure obtained from FreeRTOS website
 * https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/Embedded_Ethernet_Porting.html#xNetworkInterfaceInitialise
 * and the TivaWare Peripheral Driver Library, Chapter 10
 * http://www.ti.com/lit/ug/spmu298d/spmu298d.pdf
 * Modified by Andrew Kuklinski
 */

#include "networkinterface.h"

BaseType_t xNetworkInterfaceInitialise( void )
{
    const char socketTaskName[11] = "Socket Task";
    xTaskCreate(vSocketTask, socketTaskName, 512, NULL, 1, &socketTask);
    vTaskDelay(200);
    eth_MAC_init();

    return pdPASS;
}


void eth_MAC_init( void )
{
    uint32_t ui32User0, ui32User1, ui32Loop;
    uint8_t ui8PHYAddr;
    uint8_t pui8MACAddr[6];

    static const uint8_t ucIPAddress[ 4 ] = { 10, 10, 10, 200 };
    static const uint8_t ucNetMask[ 4 ] = { 255, 0, 0, 0 };
    static const uint8_t ucGatewayAddress[ 4 ] = { 10, 10, 10, 1 };

    /* The following is the address of an OpenDNS server. */
    //change this to the IP address of the BBG
    static const uint8_t ucDNSServerAddress[ 4 ] = { 208, 67, 222, 222 };

    //read mac address from the user registers
    FlashUserGet(&ui32User0, &ui32User1);
    if((ui32User0 == 0xffffffff) || (ui32User1 == 0xffffffff))
    {
        //if here then MAC address not programmed into hardware
        printf("Error with MAC address hardware\n");
        while(1)
        {

        }
    }

    //program MAC address into the ethernet controller registers
    pui8MACAddr[0] = ((ui32User0 >> 0) & 0xff);
    pui8MACAddr[1] = ((ui32User0 >> 8) & 0xff);
    pui8MACAddr[2] = ((ui32User0 >> 16) & 0xff);
    pui8MACAddr[3] = ((ui32User1 >> 0) & 0xff);
    pui8MACAddr[4] = ((ui32User1 >> 8) & 0xff);
    pui8MACAddr[5] = ((ui32User1 >> 16) & 0xff);

    /*******************************************************************
     * can use this to verify the existence of the TIVA board!!!!
     ******************************************************************/
    //enable and reset the ethernet modules
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EMAC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EPHY0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_EMAC0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_EPHY0);

    //waiting for the MAC to be ready
    printf("MAC Hardware initializing, please wait");
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EMAC0))
    {
    }

    //configure for use with the internal PHY, network physical interface device
    ui8PHYAddr = 0;
    EMACPHYConfigSet(EMAC0_BASE, (EMAC_PHY_TYPE_INTERNAL | EMAC_PHY_INT_MDIX_EN | EMAC_PHY_AN_100B_T_FULL_DUPLEX));

    //reset MAC in order to latch the PHY configurations as above
    EMACReset(EMAC0_BASE);

    //initialize the MAC and set the DMA mode
    EMACInit(EMAC0_BASE, f_sysclk, EMAC_BCONFIG_MIXED_BURST | EMAC_BCONFIG_PRIORITY_FIXED, 4, 4, 0);

    //MAC config options
    EMACConfigSet(EMAC0_BASE, (EMAC_CONFIG_FULL_DUPLEX | EMAC_CONFIG_CHECKSUM_OFFLOAD | EMAC_CONFIG_7BYTE_PREAMBLE |
                               EMAC_CONFIG_IF_GAP_96BITS | EMAC_CONFIG_USE_MACADDR0 |
                               EMAC_CONFIG_SA_FROM_DESCRIPTOR | EMAC_CONFIG_BO_LIMIT_1024),
                               (EMAC_MODE_RX_STORE_FORWARD | EMAC_MODE_TX_STORE_FORWARD | EMAC_MODE_TX_THRESHOLD_64_BYTES |
                               EMAC_MODE_RX_THRESHOLD_64_BYTES), 0);


    //intitialize the ethernet DMA descriptors
    //**********************************************this needs to be written as a seperate function
    InitDescriptors(EMAC0_BASE);

    //program HW with MAC address
    EMACAddrSet(EMAC0_BASE, 0, pui8MACAddr);

    //wait for PHY layer to become active, read the contents of the PHY register
    printf("Waiting for the PHY layer to become active\n");
    while((EMACPHYRead(EMAC0_BASE, ui8PHYAddr, EPHY_BMSR) & EPHY_BMSR_LINKSTAT) == 0)
    {
    }

    //MAC address filtering options.  receive all broadcast and multicast packets along with those address for us
    //?????is this necessary for our application????????
    EMACFrameFilterSet(EMAC0_BASE, (EMAC_FRMFILTER_SADDR | EMAC_FRMFILTER_PASS_MULTICAST | EMAC_FRMFILTER_PASS_NO_CTRL));

    //clear any pending interrupts
    EMACIntClear(EMAC0_BASE, EMACIntStatus(EMAC0_BASE, false));

    //mark receive descriptors as available to the DMA  to start receive process
    for(ui32Loop = 0; ui32Loop < NUM_RX_DESCRIPTORS; ui32Loop++)
    {
        g_psRxDescriptor[ui32Loop].ui32CtrlStatus |= DES0_RX_CTRL_OWN;
    }

    //enable the ethernet MAC transmitter and receiver
    EMACTxEnable(EMAC0_BASE);
    EMACRxEnable(EMAC0_BASE);

    //enable the ethernet interrupt
    IntEnable(INT_EMAC0);

    //enable the ethernet RX packet interrup source
    EMACIntEnable(EMAC0_BASE, EMAC_INT_RECEIVE);

    FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, pui8MACAddr);// ucMACAddress);
}

void InitDescriptors(uint32_t ui32Base)
{
    uint32_t ui32Loop;

    //TX and RX descriptors describe a single ethernet DMA buffer
    /*leaving the buffer pointer and size empty and the OWN bit clear
     * since we have not set up any tranmissions yet.
     */
    //init transmit descriptors
    for(ui32Loop = 0; ui32Loop < NUM_TX_DESCRIPTORS; ui32Loop++)
    {
        g_psTxDescriptor[ui32Loop].ui32Count = DES1_TX_CTRL_SADDR_INSERT;
        g_psTxDescriptor[ui32Loop].DES3.pLink = (ui32Loop == (NUM_TX_DESCRIPTORS - 1)) ?
                                            g_psTxDescriptor : &g_psTxDescriptor[ui32Loop + 1];
        g_psTxDescriptor[ui32Loop].ui32CtrlStatus = (DES0_TX_CTRL_LAST_SEG | DES0_TX_CTRL_FIRST_SEG |
                                                    DES0_TX_CTRL_INTERRUPT | DES0_TX_CTRL_CHAINED |
                                                    DES0_TX_CTRL_IP_ALL_CKHSUMS);
    }

    //init receive descriptors
    for(ui32Loop = 0; ui32Loop < NUM_RX_DESCRIPTORS; ui32Loop++)
    {
        g_psRxDescriptor[ui32Loop].ui32CtrlStatus = 0;
        g_psRxDescriptor[ui32Loop].ui32Count = (DES1_RX_CTRL_CHAINED | (RX_BUFFER_SIZE << DES1_RX_CTRL_BUFF1_SIZE_S));
        g_psRxDescriptor[ui32Loop].pvBuffer1 = g_ppui8RxBuffer[ui32Loop];
        g_psRxDescriptor[ui32Loop].DES3.pLink = (ui32Loop == (NUM_RX_DESCRIPTORS - 1)) ?
                                                g_psRxDescriptor : &g_psRxDescriptor[ui32Loop + 1];
    }

    //set the descriptor pinters in the HW
    EMACRxDMADescriptorListSet(ui32Base, g_psRxDescriptor);
    EMACTxDMADescriptorListSet(ui32Base, g_psTxDescriptor);

    /*start for the beginning of both descriptor chains
     * setting to the last descriptor since its incremented before use
     */
    g_ui32RxDescIndex = 0;
    g_ui32TxDescIndex = NUM_TX_DESCRIPTORS - 1;

}

int32_t ProcessReceivedPacket(void)
{
    int_fast32_t i32FrameLen;

    //by default we assume we received a bad frame
    i32FrameLen = 0;

    //make sure that we own the receive packet, check the descriptor struct
    if(!(g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus & DES0_RX_CTRL_OWN))
    {
        //yes we own it so check to see if it contains a valid frame
        if(!(g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus & DES0_RX_STAT_ERR))
        {
            /*the frame is valid, check the last descriptor flag is set.  we sized the receive buffer such that it can
             * always hold a valid frame so this flag should never be clear at the point...
             */
            if(g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus & DES0_RX_STAT_LAST_DESC)
            {
                //get size of frame
                i32FrameLen = ((g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus & DES0_RX_STAT_FRAME_LENGTH_M) >>
                                DES0_RX_STAT_FRAME_LENGTH_S);
                //pass the received buffer up to the application to handle
                //???????application as in TCP/IP or user program??????
                //this might have to be written
                //ApplicationProcessFrame(i32FrameLen, g_psRxDescriptor[g_ui32RxDescIndex].pvBuffer1);
            }
        }
        //hand the descriptor back to the HW
        g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus = DES0_RX_CTRL_OWN;

        //move to the next descriptor
        g_ui32RxDescIndex++;
        if(g_ui32RxDescIndex == NUM_RX_DESCRIPTORS)
        {
            g_ui32RxDescIndex = 0;
        }
    }
    //return the frame length
    return(i32FrameLen);
}

void EthernetIntHandler(void)
{
    uint32_t ui32Temp;

    //read and clear in the interrupt
    ui32Temp = EMACIntStatus(EMAC0_BASE, true);
    EMACIntClear(EMAC0_BASE, ui32Temp);

    //check to see if an RX interrupt has occurred
    if(ui32Temp & EMAC_INT_RECEIVE)
    {
        // doesn't actually process packet, this should just move the fifo over incrementing the descriptor
        ProcessReceivedPacket();
    }


    // portYIELDFromISR(the task of interest)
    // defer the processing of the packet to a task
}

//this is the same at eth_tx from the class slides
//PacketTransmit(pxDescriptor->pucEthernetBuffer, pxDscriptor->xDataLength)
/*static int32_t PacketTransmit(uint8_t *pui8Buf, int32_t i32BufLen)
{
    //wait for transmit descriptor to free up
    while(g_psTxDescriptor[g_ui32TxDescIndex].ui32CtrlStatus & DES0_TX_CTRL_OWN)
    {
    }

    //move to the next descriptor
    g_ui32TxDescIndex++;
    if(g_ui32TxDescIndex == NUM_TX_DESCRIPTORS)
    {
        g_ui32TxDescIndex = 0;
    }

    //fill in the packet size and pointer and tell the trnamitter to start work
    g_psTxDescriptor[g_ui32TxDescIndex].ui32Count = (uint32_t)i32BufLen;
    g_psTxDescriptor[g_ui32TxDescIndex].pvBuffer1 = pui8Buf;
    g_psTxDescriptor[g_ui32TxDescIndex].ui32CtrlStatus = (DES0_TX_CTRL_LAST_SEG | DES0_TX_CTRL_FIRST_SEG |
                                                          DES0_TX_CTRL_INTERRUPT | DES0_TX_CTRL_IP_ALL_CKHSUMS |
                                                          DES0_TX_CTRL_CHAINED | DES0_TX_CTRL_OWN);

    //tell DMA to reacquire the descriptor now that we have filled it in.
    EMACTxDMAPollDemand(EMAC0_BASE);

    //return the number of bytes sent
    return(i32BufLen);

}*/

void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
    uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
    static BaseType_t xTasksAlreadyCreated = pdFALSE;
    int8_t cBuffer[ 16 ];

        /* Check this was a network up event, as opposed to a network down event. */
        if( eNetworkEvent == eNetworkUp )
        {
            /* Create the tasks that use the IP stack if they have not already been
            created. */
            if( xTasksAlreadyCreated == pdFALSE )
            {
                /*
                 * Create the tasks here.
                 */

                xTasksAlreadyCreated = pdTRUE;
            }

            /* The network is up and configured.  Print out the configuration,
            which may have been obtained from a DHCP server. */
            FreeRTOS_GetAddressConfiguration( &ulIPAddress,
                                              &ulNetMask,
                                              &ulGatewayAddress,
                                              &ulDNSServerAddress );

            /* Convert the IP address to a string then print it out. */
            FreeRTOS_inet_ntoa( ulIPAddress, cBuffer );
            printf( "IP Address: %s\r\n", cBuffer );

            /* Convert the net mask to a string then print it out. */
            FreeRTOS_inet_ntoa( ulNetMask, cBuffer );
            printf( "Subnet Mask: %s\r\n", cBuffer );

            /* Convert the IP address of the gateway to a string then print it out. */
            FreeRTOS_inet_ntoa( ulGatewayAddress, cBuffer );
            printf( "Gateway IP Address: %s\r\n", cBuffer );

            /* Convert the IP address of the DNS server to a string then print it out. */
            FreeRTOS_inet_ntoa( ulDNSServerAddress, cBuffer );
            printf( "DNS server IP Address: %s\r\n", cBuffer );
        }
}

BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t * const pxDescriptor, BaseType_t xReleaseAfterSend )
{

    //SendData( pxDescriptor->pucBuffer, pxDescriptor->xDataLength );

    if( xReleaseAfterSend != pdFALSE )
    {
        vReleaseNetworkBufferAndDescriptor( pxDescriptor );
    }
    return pdTRUE;
}

void vSocketTask(void *pvParameters)
{
    printf("inside socket task\n");


    //initializing the MAC layer for interface to the TPC/IP layer

    /*Socket_t xclientHandle;
    struct freertos_sockaddr xclientAddress;
    size_t xbytesSent = 0;
    size_t xbytesTOSEND = 256;
    uint16_t port1 = 8090;
    char ipc_mess[xbytesTOSEND];
    sprintf(ipc_mess,"%s", "This message is bound for the BBG SERVER!!");

    xclientAddress.sin_port = FreeRTOS_htons(port1);
    //might need to use FreeRTOS_inet_addr() to conform to Berkley sockets
    xclientAddress.sin_addr = FreeRTOS_inet_addr_quick(192, 168,7,2);

    xclientHandle = FreeRTOS_socket(FREERTOS_AF_INET,FREERTOS_SOCK_STREAM,FREERTOS_IPPROTO_TCP);
    if(xclientHandle == FREERTOS_INVALID_SOCKET)
    {
        printf("Error Creating Socket: %s\n", "FREERTOS_INVALID_SOCKET");
    }

    if(FreeRTOS_connect(xclientHandle, &xclientAddress, sizeof(xclientAddress))==0)
    {
        xbytesSent = FreeRTOS_send(xclientHandle, &ipc_mess, xbytesTOSEND,0);
        if(xbytesSent == -pdFREERTOS_ERRNO_ENOTCONN)
        {
            printf("error on send: %s\n", "FREERTOS_ERRNO_ENOTCONN");
        }
    }

    FreeRTOS_shutdown(xclientHandle, FREERTOS_SHUT_RDWR);
    while(FreeRTOS_recv(xclientHandle, ipc_mess, xbytesTOSEND,0) >= 0)
    {
        //vTaskDelay(pdTICKS_TO_MS(250));
    }
    FreeRTOS_closesocket(xclientHandle);*/

    while(1);

}






