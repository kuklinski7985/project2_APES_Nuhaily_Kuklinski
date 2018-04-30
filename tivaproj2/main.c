/**
 * @file main.c
 * @author Adam Nuhaily and Andrew Kuklinski
 * @date 4/29/2018
 * @brief main function for project 2 using FreeRTOS and TivaWare
 */

#include "main.h"

uint8_t led_state;
uint32_t f_sysclk;

//Creating task
TaskHandle_t masterTask;
TaskHandle_t terminalTask;
TaskHandle_t gpioTask;
TaskHandle_t heartbeatTask;
TaskHandle_t RFIDTask;
TaskHandle_t CameraTask;
TaskHandle_t TerminalLogging;

QueueHandle_t ipc_msg_queue;
QueueHandle_t uart_term_rx_queue;
QueueHandle_t logging_queue;

heartbeat_t hb_terminal;
heartbeat_t hb_gpio;
heartbeat_t hb_rfid;

int hb_terminal_task_timedout;
int hb_gpio_task_timedout;
int hb_rfid_task_timedout;

const char terminalTaskName[17] = "Terminal Rx Task";
const char masterTaskName[12] = "Master Task";
const char gpioTaskName[10] = "GPIO Task";
const char heartbeatTaskName[15] = "Heartbeat Task";
const char RFIDTaskName[9] = "RFID Task";
const char CameraTaskName[11] = "Camera Task";
const char TermLogName[21] = "Terminal Logging Task";

//global variables for the all task heartbeat
uint8_t tiva_sec = 0;
uint8_t tiva_min = 0;
SemaphoreHandle_t xtimestamp_sema;

//uint8_t rfid_sec = 0;
SemaphoreHandle_t xrfid_sema;
char timeString[9];
const int hb_timeout = 10;

/**
 * @brief Main function. Set system clock and configure peripherals
 *
 */
int main(void)
{

    xtimestamp_sema = xSemaphoreCreateBinary();
    TimerHandle_t timeStampTimer = xTimerCreate("TimeStamp Timer", pdMS_TO_TICKS(1000),
                                                pdTRUE, (void*)0, vTimeStampCallBack);
    xTimerStart(timeStampTimer,0);

    xrfid_sema = xSemaphoreCreateBinary();

    led_state = 0;
    hb_terminal = 0;
    hb_gpio = 0;
    hb_terminal_task_timedout = 0;
    hb_gpio_task_timedout = 0;
    hb_rfid_task_timedout = 0;

    // Set system clock to 120MHz
    f_sysclk = MAP_SysCtlClockFreqSet( (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | \
            SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480) , CLOCK_120MHZ);

    // Configure GPIO for LED
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // Enable the GPIO pins for the LED (PN0).
    //ROM_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4);

    //ROM_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0b1);
    ROM_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0b00);

    // Initialize LED state
    //ROM_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0); // Probably not strictly necessary

    // Configure UART
    ConfigureUART();

    // Enable processor interrupts
    ROM_IntMasterEnable();

    // Try to create queue, catch error if fails
    if( (ipc_msg_queue = xQueueCreate(QUEUE_LENGTH_MAX, \
                                      QUEUE_ELEMENT_SIZE_MAX) ) == NULL)
    {
        UARTprintf("Error creating IPC queue.\n");
        while(1);
    }

    if( (uart_term_rx_queue = xQueueCreate(QUEUE_LENGTH_MAX, \
                                           QUEUE_ELEMENT_SIZE_MAX) ) == NULL)
    {
            // yeah I know we're bypassing the UART queue. We have to!
            // It's not created yet!
        UARTprintf("Error creating UART terminal queue.\n");

        // Hang program
        while(1);
    }

    if( (logging_queue = xQueueCreate(QUEUE_LENGTH_MAX, \
                                      QUEUE_ELEMENT_SIZE_MAX) ) == NULL)
    {
        UARTprintf("Error creating logging queue.\n");
        while(1);
    }



    /*******************************************************************************************************
     * code for testing the connection to the BBG
    uint8_t i;
    uint8_t mess[15] = "6 to 3 testing.";
    //uint8_t recv[32] = {'x'};
    UARTprintf("\nmessage from UART4 to UART3\n");
    for(i =0; i < 15;i++)
    {
        ROM_UARTCharPutNonBlocking(UART4_BASE,mess[i]);
    }


    *******************************************************************************************************/

    xInitThreads();

    // Print welcome message

    vTaskStartScheduler();

    // We'll never get here in this implementation as our threads never exit
    while(1)
    {
    }

}

int xInitThreads(void)
{
    // Create tasks and schedule
    xTaskCreate(vMasterTask, masterTaskName, 512, NULL, 1, &masterTask);
    xTaskCreate(vUARTRxTerminalTask, terminalTaskName, 1024, NULL, 1, &terminalTask);
    xTaskCreate(vGPIOTask, gpioTaskName, 1024, NULL, 1, &gpioTask);
    xTaskCreate(vHeartbeatTask, heartbeatTaskName, 512, NULL, 1, &heartbeatTask);
    xTaskCreate(vRFIDTask, RFIDTaskName, 512, NULL, 1, &RFIDTask);
    //xTaskCreate(vCameraTask, CameraTaskName, 512, NULL, 1, &CameraTask);
    xTaskCreate(vTerminalLogging, TermLogName, 512, NULL, 1, &TerminalLogging);
    //xNetworkInterfaceInitialise();              //supposed to be the functionality for TCP/IP connection but never got it working
    return 0;
}

/**
 * @brief Configure UART for 115200 8-n-1
 *
 */
void ConfigureUART(void)
{
    /*
     * Explanation for Commented code:
     * The UART initialization code that is commented out below is necessary for the operation of the camera and connection to
     * the BBG.  Since these functions are not currently operational, they must be commented in order to keep the rest of the code
     * from freezing.  If an initialized UART does not have a connection, everything hands.  A solution to this would be to
     * to start the internal pull up resistors once the UARTS have been started.
     */
    // Enable the GPIO Peripheral used by the UART.

    //UART0, terminal output for comms
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);      //connection to user interface terminal
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    ROM_IntEnable(INT_UART0);
    ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    UARTStdioConfig(0, UARTBAUDRATE, f_sysclk);

    //UART6, RFID connections, pins PP0 and PP1
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6);
    ROM_GPIOPinConfigure(GPIO_PP0_U6RX);
    ROM_GPIOPinConfigure(GPIO_PP1_U6TX);
    ROM_GPIOPinTypeUART(GPIO_PORTP_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    ROM_UARTConfigSetExpClk(UART6_BASE, f_sysclk, UARTBAUDRATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    ROM_IntEnable(INT_UART6);
    ROM_UARTIntEnable(UART6_BASE, UART_INT_RX | UART_INT_RT);

    //UART5, camera connections, pins
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);      //connection to camera
    ROM_GPIOPinConfigure(GPIO_PC6_U5RX);
    ROM_GPIOPinConfigure(GPIO_PC7_U5TX);
    ROM_GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_7);
    ROM_UARTConfigSetExpClk(UART5_BASE, f_sysclk, 38400, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));
    /****uncomment the following two lines to enable****/
    //ROM_IntEnable(INT_UART5);
    //ROM_UARTIntEnable(UART5_BASE, UART_INT_RX);// | UART_INT_RT);

    //UART3, connections for inter-board communication to BBG
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART3);      //connection to the BBG
    ROM_GPIOPinConfigure(GPIO_PA4_U3RX);
    ROM_GPIOPinConfigure(GPIO_PA5_U3TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    ROM_UARTConfigSetExpClk(UART3_BASE, f_sysclk, 38400, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));
    /****uncomment the following two lines to enable****/
    //ROM_IntEnable(INT_UART3);
    //ROM_UARTIntEnable(UART3_BASE, UART_INT_RX | UART_INT_RT);

}

/**
 * @brief Handle IPC communications, message interpretation, external interface
 *
 */
void vMasterTask(void* pvParameters)
{
    // This task waits on the IPC message queue for something to happen
    // When something appears in the queue it pops it and processes it
    // Anything that needs to happen concurrently needs a separate task made
    //  e.g. sensors, heartbeat, etc

    char snd_str[QUEUE_ELEMENT_SIZE_MAX];           //main task queue, should send out to location stored in destination
    ipcmessage_t master_IPC_mess;

    getTimeStamp(&master_IPC_mess);
    master_IPC_mess.type = MSG_INFO;
    master_IPC_mess.source = TASK_MAIN;
    master_IPC_mess.destination = TASK_LOGGING;
    strcpy(master_IPC_mess.payload, "Master Task Initialized");
    build_ipc_msg(master_IPC_mess, snd_str);
    xQueueSend(ipc_msg_queue, snd_str, portMAX_DELAY);

    char cmdbuf[QUEUE_ELEMENT_SIZE_MAX];
    ipcmessage_t last_queue_msg;    // name this something a little better

    //vPrintTerminalMenu();
    //vPrintTerminalPrompt();

    while(1)
    {
      //  UARTprintf("Main task waiting for IPC message.\n");
        // Block on queue waiting for message (do we need to alter priority of this operation?)
        //while(uxQueueMessagesWaiting(ipc_msg_queue) == 0);

        // need to add err check and timeout?
        xQueueReceive(ipc_msg_queue, cmdbuf, portMAX_DELAY);
       // UARTprintf("Main task got IPC message.\n");
        // process data in cmdbuf:
        // -check if cmdbuf is null or is in wrong format (delimiters)
        //      +typecast to struct instead? hm
        // -do delimiter translation from BBG code (just copy that code over)
        // -figure out what to do with it and do it
        //      +does the "do it" portion need a separate task?

        // do null string check and correct format check- look for correct # of delimiters
        // checkMsgFormat();
        // or maybe have the decipher function automatically detect null chars in wrong place
       // UARTprintf("Main task received: %s\n", cmdbuf);
        decipher_ipc_msg(cmdbuf, &last_queue_msg);   // would be great if this had a return status
       //UARTprintf("source: %d and destination: %d\n", last_queue_msg.source, last_queue_msg.destination);
        // now switch depending on:
        // 1) destination- put it on the appropriate queue
        // 2) if for main, check source
        // 3) then within source, check type
        switch(last_queue_msg.destination)
        {
            case TASK_MAIN:
                // messages destined for this task
                switch(last_queue_msg.source)
                {
                    case TASK_TERMINAL:
                        // decode input and act
                       // UARTprintf("Main task detects terminal input: %s\n", last_queue_msg.payload);
                        //xProcessTerminalInput(last_queue_msg.payload); // come up with some kind of return status
                        break;
                    case TASK_HB:
                        xProcessHBInput(last_queue_msg.payload);
                        break;
                    case TASK_MAIN:
                        xQueueSend(logging_queue, cmdbuf, portMAX_DELAY);
                    default:
                        break;
                }

                break;
            case TASK_TERMINAL:
                // messages destined for terminal UART port (display)
                // push to terminal queue
                UARTprintf("%s", last_queue_msg.payload);
                break;

            case TASK_NONE:
            case TASK_HB:
            case TASK_SOCKET:
            case TASK_UART:
            case TASK_LOGGING:
                xQueueSend(logging_queue, cmdbuf, portMAX_DELAY);

                break;
            default:
                // print to UART terminal that an unrecognized message was detected
                break;
        }
    }
}

int xProcessTerminalInput(char* input)
{
    if(input[0] == '\0')
    {
        UARTprintf("Invalid input.\n");
        return -1;  // need some kind of proper error return status
    }

    switch(input[0])
    {
        case 'l':
        case 'L':
            // toggle LED
            // send notify to GPIO task
            xTaskNotify(gpioTask, 1, eSetBits);
            break;
        case 'm':
        case 'M':
            UARTprintf("\n");
            vPrintTerminalMenu();
            break;
        default:
            break;
    }

    vPrintTerminalPrompt();

    return 0;
}

int xProcessHBInput(char* input)
{
    location_t task_timed_out = TASK_NONE;
    char snd_str[QUEUE_ELEMENT_SIZE_MAX];           //main task queue, should send out to location stored in destination
    ipcmessage_t hb_IPC_mess;

    if(input[0] == '\0')
    {
        UARTprintf("Invalid input.\n");
        return -1;  // need some kind of proper error return status
    }

    task_timed_out = (location_t)(input[0] - '0');  // use your code from PES for this instead
    //UARTprintf("char: %c location: %d\n", input[0], task_timed_out);
    switch(task_timed_out)
    {
        case TASK_TERMINAL:
            // maybe turn on or blink an LED
            xTaskNotify(gpioTask, 1, eSetBits);
            break;
        case TASK_GPIO:
            getTimeStamp(&hb_IPC_mess);
            hb_IPC_mess.type = MSG_TERMINATE;
            hb_IPC_mess.source = TASK_HB;
            hb_IPC_mess.destination = TASK_LOGGING;
            strcpy(hb_IPC_mess.payload, "GPIO Task Timed out");
            build_ipc_msg(hb_IPC_mess, snd_str);
            xQueueSend(ipc_msg_queue, snd_str, portMAX_DELAY);
            UARTprintf("\nGPIO Task timed out.\n"); // add timestamp -- actually, should this board know or care what time it is?
            break;
        case TASK_RFID:
            getTimeStamp(&hb_IPC_mess);
            hb_IPC_mess.type = MSG_TERMINATE;
            hb_IPC_mess.source = TASK_HB;
            hb_IPC_mess.destination = TASK_LOGGING;
            strcpy(hb_IPC_mess.payload, "RFID Task Timed out");
            build_ipc_msg(hb_IPC_mess, snd_str);
            xQueueSend(ipc_msg_queue, snd_str, portMAX_DELAY);
            UARTprintf("\nRFID Task timed out\n");
            break;
        default:
            break;
    }

    return 0;
}

void vPrintTerminalMenu(void)
{
    UARTprintf("Terminal menu: \n");
    UARTprintf("(L) LED on\n");
    UARTprintf("(M) Print this menu\n");
 //   UARTprintf("> \n");
}

void vPrintTerminalPrompt(void)
{
    //UARTprintf("Enter command (M for menu): ");
   // UARTprintf(".");
}

/* old task1 code:
      // Turn on LED
    HWREGBITW(&led_state, 0) = 0b01;
    ROM_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, led_state);

    // Configure Timer0
    ConfigureTimer0();
*/

/**
* @brief Interrupt handler for communications from the user UART terminal
*
*/
void UARTTerminalIntHandler()
{
    // either use a fromISR function or use a notify to another task to read from the buffer and process
    char rxbuf[DEFAULT_BUFFER_SIZE];
 //   UARTprintf("Entered UART Terminal ISR Handler.\n");
    UARTgets(rxbuf, DEFAULT_BUFFER_SIZE);
    xQueueSendFromISR(uart_term_rx_queue, rxbuf, NULL);
 //   UARTprintf("%s", rxbuf);    // echo
    return;
}

void UART_RFID_Handler(void)
{
    static uint8_t elements = 0;
    static uint8_t data_num_bytes=0;

    rfid_data_recv[elements] = ROM_UARTCharGet(UART6_BASE);
    //UARTprintf("recv[%d]: %02x\n",elements,rfid_data_recv[elements]);

    if(elements==2)
    {
        data_num_bytes = rfid_data_recv[2];
    }
    elements++;
    if(elements==(data_num_bytes+4))
    {
        elements=0;
        data_num_bytes=0;
        rfid_handler_exit_flag = 1;
    }
}



void vRFIDTask(void *pvParameters)
{

    char snd_str[QUEUE_ELEMENT_SIZE_MAX];           //main task queue, should send out to location stored in destination
    ipcmessage_t RFID_IPC_mess;

    getTimeStamp(&RFID_IPC_mess);
    RFID_IPC_mess.type = MSG_INFO;
    RFID_IPC_mess.source = TASK_RFID;
    RFID_IPC_mess.destination = TASK_LOGGING;
    strcpy(RFID_IPC_mess.payload, "RFID Task Init...");
    build_ipc_msg(RFID_IPC_mess, snd_str);
    xQueueSend(ipc_msg_queue, snd_str, portMAX_DELAY);

    RFID_reset();

    while(1)
    {
        hb_rfid = 0;
        RFID_seek();
        vTaskDelay(5000 / portTICK_PERIOD_MS);

    }
}


void vUARTRxTerminalTask(void* pvParameters)
{

    char snd_str[QUEUE_ELEMENT_SIZE_MAX];           //main task queue, should send out to location stored in destination
    ipcmessage_t UARTtermTask_IPC_Mess;

    getTimeStamp(&UARTtermTask_IPC_Mess);
    UARTtermTask_IPC_Mess.type = MSG_INFO;
    UARTtermTask_IPC_Mess.source = TASK_TERMINAL;
    UARTtermTask_IPC_Mess.destination = TASK_LOGGING;
    strcpy(UARTtermTask_IPC_Mess.payload, "Terminal Task Initialized");
    build_ipc_msg(UARTtermTask_IPC_Mess, snd_str);
    xQueueSend(ipc_msg_queue, snd_str, portMAX_DELAY);


    char strbuf[QUEUE_ELEMENT_SIZE_MAX];
    ipcmessage_t ipc_struct_buf;
    char ipc_string_buf[QUEUE_ELEMENT_SIZE_MAX];
    BaseType_t rtn;

    while(1)
    {
        // monitor rx queue for elements
        // if an element appears, grab it and figure out what it is
        // use a switch statement to form an ipc message struct
        // then translate that struct to a string and push to IPC queue
     //   UARTprintf("Rx Terminal task waiting for IPC message.\n");
       // while(uxQueueMessagesWaiting(uart_term_rx_queue) == 0);


                // time out after 10ms so we can reset the watchdog
       rtn = xQueueReceive(uart_term_rx_queue, strbuf, 10 /*portMAX_DELAY*/);
       hb_terminal = 0;
      // UARTprintf("Rx Terminal task received IPC message.\n");
       // user input is now contained in strbuf, figure out what's in it
       if(rtn != pdPASS)
       {
           continue;    // if there was nothing in the queue don't bother doing any processing
       }

       if(strbuf[0] == '\0')
       {
           // send message to main saying that an invalid command was received
       }
       else
       {
           // build IPC struct and send to main
           strcpy(ipc_struct_buf.timestamp, "xx:xx > ");
           ipc_struct_buf.type = MSG_DATA;
           ipc_struct_buf.source = TASK_TERMINAL;
         //  ipc_struct_buf.src_handle = (TaskHandle_t)16;//xTaskGetHandle(terminalTaskName); // apparently this function can take a long time to execute
           ipc_struct_buf.destination = TASK_MAIN;
           strcpy(ipc_struct_buf.payload, strbuf);

           // Build string from struct and send
           build_ipc_msg(ipc_struct_buf, ipc_string_buf);
          // UARTprintf("String sent to main task: %s\n", ipc_string_buf);
           xQueueSend(ipc_msg_queue, ipc_string_buf, portMAX_DELAY);
       }
    }
}

void vGPIOTask(void* pvParameters)
{
    char snd_str[QUEUE_ELEMENT_SIZE_MAX];
    ipcmessage_t gpio_IPC_mess;

    getTimeStamp(&gpio_IPC_mess);
    gpio_IPC_mess.type = MSG_INFO;
    gpio_IPC_mess.source = TASK_TERMINAL;
    gpio_IPC_mess.destination = TASK_LOGGING;
    strcpy(gpio_IPC_mess.payload, "GPIO Task Initialized");
    build_ipc_msg(gpio_IPC_mess, snd_str);
    xQueueSend(ipc_msg_queue, snd_str, portMAX_DELAY);

    uint32_t task_notification = 0; // incoming tasks
    // read a task notifier and determine what to do with which LED
    // maintain a register that stores LED states locally?
    while(1)
    {
        xTaskNotifyWait(0, ULONG_MAX, &task_notification, portMAX_DELAY);   // this thread currently times out (heartbeat) for demo purposes
        // Toggle 1 LED                                                     // change to have a timeout in the notifywait so we can reset the watchdog
        HWREGBITW(&led_state, 0) ^= 1;
        ROM_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, led_state << 0); // LED 1
        ROM_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, led_state << 1); // LED 2
        ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, led_state << 4); // LED 3
        ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, led_state << 0); // LED 4
    }
}

void vHeartbeatTask(void *pvParameters)
{
    char snd_str[QUEUE_ELEMENT_SIZE_MAX];           //main task queue, should send out to location stored in destination
    ipcmessage_t hb_IPC_mess;

    getTimeStamp(&hb_IPC_mess);
    hb_IPC_mess.type = MSG_INFO;
    hb_IPC_mess.source = TASK_TERMINAL;
    hb_IPC_mess.destination = TASK_LOGGING;
    strcpy(hb_IPC_mess.payload, "Heartbeat Task Initialized");
    build_ipc_msg(hb_IPC_mess, snd_str);
    xQueueSend(ipc_msg_queue, snd_str, portMAX_DELAY);

    TimerHandle_t hb_timer = xTimerCreate("Heartbeat Timer", pdMS_TO_TICKS(1000), pdTRUE, (void*)0, vHBTimerCallback);

    ipcmessage_t errstruct;
    char strbuf[2];
    char errmsg[QUEUE_ELEMENT_SIZE_MAX];

    //strcpy(errstruct.timestamp, "xx:xx > ");    // should we keep timestamps in the TIVA? maybe that's a server-side sort of thing
    errstruct.type = MSG_ERROR;                 // or maybe the server sends time info to the TIVA
    errstruct.source = TASK_HB;
    errstruct.destination = TASK_MAIN;

    xTimerStart(hb_timer, 0); // Start immediately

    while(1)
    {
        // increment counters, push message to queue if any go above threshold
        // other tasks continually reset counters
        // set up timer and increment heartbeats on callback
        if(hb_terminal > hb_timeout && hb_terminal_task_timedout == pdFALSE)
        {
            hb_terminal_task_timedout = pdTRUE;
            getTimeStamp(&errstruct);
            sprintf(strbuf, "%d", TASK_TERMINAL);
            strcpy(errstruct.payload, strbuf);
            build_ipc_msg(errstruct, errmsg);
            xQueueSend(ipc_msg_queue, errmsg, NULL);
        }

        if(hb_gpio > hb_timeout && hb_gpio_task_timedout == pdFALSE)
        {
            hb_gpio_task_timedout = pdTRUE;
            getTimeStamp(&errstruct);
            sprintf(strbuf, "%d", TASK_GPIO);
            strcpy(errstruct.payload, strbuf);
            build_ipc_msg(errstruct, errmsg);
            xQueueSend(ipc_msg_queue, errmsg, NULL);
        }
        if(hb_rfid > hb_timeout && hb_rfid_task_timedout == pdFALSE)
        {
            hb_rfid_task_timedout = pdTRUE;
            getTimeStamp(&errstruct);
            sprintf(strbuf, "%d", TASK_RFID);
            strcpy(errstruct.payload, strbuf);
            build_ipc_msg(errstruct, errmsg);
            xQueueSend(ipc_msg_queue, errmsg, NULL);
        }
    }
}

void vHBTimerCallback(void* pvParameters)
{
    hb_terminal++;
    hb_gpio++;
    hb_rfid++;
}

void UART_CAMERAinter_Handler(void)
{
    camera_handler_exit_flag = 0;
    static uint8_t elements = 0;
    static uint8_t recv_cmd = 0;
    static uint8_t total_recv = 0;

    uint32_t status;
    status = ROM_UARTIntStatus(UART5_BASE,true);

    while(ROM_UARTCharsAvail(UART5_BASE))
    {
        ROM_UARTIntClear(UART5_BASE, status);
        camera_data_recv[elements] = ROM_UARTCharGetNonBlocking(UART5_BASE);
        //camera_data_recv[elements] = ROM_UARTCharGet(UART4_BASE);
        UARTprintf("%c",camera_data_recv[elements]);
        if(elements==2)
        {
            recv_cmd = camera_data_recv[2];
        }
        elements++;
    }

    switch(recv_cmd){
    case 0x26:
        //total number of bytes received should be 4
        total_recv = elements +1;
        if(elements == total_recv)
        {
            elements = 0;
            camera_handler_exit_flag = 1;
            UARTprintf("Reset Command Receive COMPLETE...\n");
        }
        break;
    case 0x36:
        //total received should be 5
        total_recv = elements +5;
        if(elements == total_recv)
        {
            elements = 0;
            camera_handler_exit_flag = 1;
            UARTprintf("Stop Frame Command Receive COMPLETE...\n");
        }
        break;
    case 0x34:
        //total received should be 9
        total_recv = elements +6;
        if(elements == total_recv)
        {
            elements = 0;
            camera_handler_exit_flag = 1;
            UARTprintf("Reset Command Receive COMPLETE...\n");
        }
        break;
    case 0x31:
        //total received should be 5
        total_recv = elements +1;
        if(elements == total_recv)
        {
            elements = 0;
            camera_handler_exit_flag = 1;
            UARTprintf("Set Image Size Receive COMPLETE...\n");
        }
        break;
    case 0x11:
        //total received should be 11
        total_recv = elements+25;// + 8;
        if(elements == total_recv)
        {
            elements = 0;
            camera_handler_exit_flag = 1;
            UARTprintf("Get Version Receive COMPLETE...\n");
        }
        break;
    default:
        //returns command byte has not been received yet
        break;
    }
}

void vCameraTask(void* pvParameters)
{
    char snd_str[QUEUE_ELEMENT_SIZE_MAX];           //main task queue, should send out to location stored in destination
    ipcmessage_t camera_IPC_mess;

    getTimeStamp(&camera_IPC_mess);
    camera_IPC_mess.type = MSG_INFO;
    camera_IPC_mess.source = TASK_TERMINAL;
    camera_IPC_mess.destination = TASK_LOGGING;
    strcpy(camera_IPC_mess.payload, "Camera Task Initialized");
    build_ipc_msg(camera_IPC_mess, snd_str);
    xQueueSend(ipc_msg_queue, snd_str, portMAX_DELAY);
    //UARTprintf("Getting version...\n");
    //uint8_t get_version[] = {PROTO_SIGN_RECV, SN_NUM, CMD_GET_VERSION, NADA};
    //UARTCamera_sendCommand(get_version, GET_VERSION_LENGTH);

    while(1);
}

//Receiving data on the RX pin of UART3
//intended for use with cross board communication with the BBG
void UART3_BBG_Handler(void)
{
    uint32_t status;
    char mess;
    status = ROM_UARTIntStatus(UART3_BASE, true);
    ROM_UARTIntClear(UART3_BASE, status);

    while(ROM_UARTCharsAvail(UART3_BASE))
    {
        mess = ROM_UARTCharGetNonBlocking(UART3_BASE);
        UARTprintf("[]: %c\n",mess);
    }
}


void vTerminalLogging(void* pvParameters)
{
    ipcmessage_t log_IPC_msg;
    char recv_str[QUEUE_ELEMENT_SIZE_MAX];
    while(1)
    {
        xQueueReceive(logging_queue, recv_str, portMAX_DELAY);
        decipher_ipc_msg(recv_str, &log_IPC_msg);
        UARTprintf("%s",log_IPC_msg.timestamp);
        UARTprintf(" | %s\n",log_IPC_msg.payload);
    }

}

void vTimeStampCallBack( void* pvParameters )
{
    xSemaphoreTake(xtimestamp_sema,0);
    tiva_sec++;
    if(tiva_sec == 60)
    {
        tiva_sec=0;
        tiva_min++;
    }
    xSemaphoreGive(xtimestamp_sema);
}


/*void vRFIDTimerCallback( void* pvParameters )
{
    xSemaphoreTake(xrfid_sema,0);
    rfid_sec++;
    xSemaphoreGive(xrfid_sema);


}*/



