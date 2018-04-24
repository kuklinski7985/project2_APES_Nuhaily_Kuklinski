/**
 * @file main.c
 * @author Adam Nuhaily
 * @date 4/8/2018
 * @brief Homework 5 problem 4 main source file
 *
 * This program is intended for a TiVA board TM4C1294. It blinks an LED at 2Hz and outputs
 * a counter over UART (115200 8-n-1) using a multi task implementation.
 */

#include "main.h"

uint8_t led_state;
uint32_t f_sysclk;

TaskHandle_t masterTask;
TaskHandle_t terminalTask;
TaskHandle_t gpioTask;
TaskHandle_t heartbeatTask;
TaskHandle_t RFIDTask;

QueueHandle_t ipc_msg_queue;
QueueHandle_t uart_term_rx_queue;

heartbeat_t hb_terminal;
heartbeat_t hb_gpio;

int hb_terminal_task_timedout;
int hb_gpio_task_timedout;

const char terminalTaskName[17] = "Terminal Rx Task";
const char masterTaskName[12] = "Master Task";
const char gpioTaskName[10] = "GPIO Task";
const char heartbeatTaskName[15] = "Heartbeat Task";
const char RFIDTaskName[9] = "RFID Task";



const int hb_timeout = 100;

/**
 * @brief Main function. Set system clock and configure peripherals
 *
 */
int main(void)
{
    led_state = 0;
    hb_terminal = 0;
    hb_gpio = 0;
    hb_terminal_task_timedout = 0;
    hb_gpio_task_timedout = 0;

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
    //xNetworkInterfaceInitialise();
    return 0;
}

/**
 * @brief Configure UART for 115200 8-n-1
 *
 */
void ConfigureUART(void)
{
    // Enable the GPIO Peripheral used by the UART.
    //should include UART0 and UART3
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);

    // Enable UART0 module
    //include UART3 module
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6);



    // Configure GPIO Pins for UART mode.
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);

    ROM_GPIOPinConfigure(GPIO_PP0_U6RX);
    ROM_GPIOPinConfigure(GPIO_PP1_U6TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    ROM_GPIOPinTypeUART(GPIO_PORTP_BASE, GPIO_PIN_0 | GPIO_PIN_1);


    //
    // Enable the UART interrupt.
    //

    ROM_UARTConfigSetExpClk(UART6_BASE, f_sysclk, UARTBAUDRATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
    ROM_IntEnable(INT_UART0);
    ROM_IntEnable(INT_UART6);
    ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    ROM_UARTIntEnable(UART6_BASE, UART_INT_RX | UART_INT_RT);

    // Initialize the UART for console I/O.
    //UARTBAUDRATE should be changed back to 115200, #define in main.h
    UARTStdioConfig(0, UARTBAUDRATE, f_sysclk);

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

    char cmdbuf[QUEUE_ELEMENT_SIZE_MAX];
    ipcmessage_t last_queue_msg;    // name this something a little better

    vPrintTerminalMenu();
    vPrintTerminalPrompt();

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
            // send a message
            UARTprintf("\nGPIO Task timed out.\n"); // add timestamp -- actually, should this board know or care what time it is?
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
    UARTprintf("Enter command (M for menu): ");
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

void UARTRFID_send(char * RFID_buffer, uint32_t byteCount)
{
    int i =0;
    for(i =0; i<byteCount;i++)
    {
        ROM_UARTCharPut(UART6_BASE, RFID_buffer[i]);
    }
}

void vRFIDTask(void *pvParameters)
{
    //write the proper baud rate
    //reset the module
    //wait for response from module with version information
    //timeout if not received in 10 seconds
    //push information to the logger via IPC
    //put module into seek mode
    //wait for a response
    //when received, print the received infromation
    //push information to the logger via IPC
    //put back into seek mode, wait for another chip read
    //uint8_t i = 0;
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
    vTaskDelay(xDelay);

    RFID_reset();
    //RFID_setbaud(115200);
    RFID_seek();



    while(1)
    {
        //RFID_seek();
    }
}

/**
 * @brief UART task, including task notification handler and UART interface
 *  maybe get rid of this and just have main do UARTprintf by itself without using a queue and task for tx
 *
 */
void vUARTRxTerminalTask(void* pvParameters)
{
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

/*
 * // struct to define messages passed around to all parts of the system
typedef struct ipcmessage {
  char timestamp[10];
  message_t type;                   // message identifier
  location_t source;                // where message originates from
  char src_handle[TASKNAME_SIZE];   // handle of process creating the message
  location_t destination;           // final destination for message
  char payload[DEFAULT_BUF_SIZE];   // message to transmit
} ipcmessage_t;
 */

void vGPIOTask(void* pvParameters)
{
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
    TimerHandle_t hb_timer = xTimerCreate("Heartbeat Timer", pdMS_TO_TICKS(1000), pdTRUE, (void*)0, vHBTimerCallback);

    ipcmessage_t errstruct;
    char strbuf[2];
    char errmsg[QUEUE_ELEMENT_SIZE_MAX];
    strcpy(errstruct.timestamp, "xx:xx > ");    // should we keep timestamps in the TIVA? maybe that's a server-side sort of thing
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
            sprintf(strbuf, "%d", TASK_TERMINAL);
            strcpy(errstruct.payload, strbuf);
            build_ipc_msg(errstruct, errmsg);
            xQueueSend(ipc_msg_queue, errmsg, NULL);
        }

        if(hb_gpio > hb_timeout && hb_gpio_task_timedout == pdFALSE)
        {
            hb_gpio_task_timedout = pdTRUE;
            sprintf(strbuf, "%d", TASK_GPIO);
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
}






















