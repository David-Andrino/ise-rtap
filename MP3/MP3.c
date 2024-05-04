#include "MP3.h"

#include "Driver_USART.h"
#include "cmsis_os2.h"

#define THREAD_FLAG_USART_DONE 0x01U

#define MAX_MSG   8
#define USART_LEN 8

extern ARM_DRIVER_USART  Driver_USART6;
static ARM_DRIVER_USART* USARTdrv = &Driver_USART6;

static void USART_Callback(uint32_t event);
static void USART_Init(ARM_DRIVER_USART* drv);

static uint8_t USARTcmd[8] = {0x7E, 0xFF, 0x06, 0x00, 0x00, 0x00, 0x00, 0xEF};

osThreadId_t       MP3_Thread;
osMessageQueueId_t MP3Queue;

void ThreadMP3(void* argument);

int Init_MP3(void) {
    MP3_Thread = osThreadNew(ThreadMP3, NULL, NULL);
    if (MP3_Thread == NULL) {
        return (-1);
    }

    osMessageQueueAttr_t attrs = { .name = "MP3Queue" };
    MP3Queue = osMessageQueueNew(MAX_MSG, sizeof(mp3Msg_t), &attrs);
    if (MP3Queue == NULL) {
        return (-1);
    }

    USART_Init(USARTdrv);

    return (0);
}

void ThreadMP3(void* argument) {
    // Set volume to max (30)
    USARTcmd[3] = 0x06;
    USARTcmd[6] = 0x1E;
    USARTdrv->Send(USARTcmd, USART_LEN);
    osThreadFlagsWait(THREAD_FLAG_USART_DONE, osFlagsWaitAny, osWaitForever);

    USARTcmd[3] = 0;
    USARTcmd[6] = 0;

    mp3Msg_t msg = {0};
    while (1) {
        osMessageQueueGet(MP3Queue, &msg, NULL, osWaitForever);
        switch (msg) {
            case MP3_NEXT_SONG:
                USARTcmd[3] = 0x01;
                break;
            case MP3_PREV_SONG:
                USARTcmd[3] = 0x02;
                break;
            case MP3_VOL_UP:
                USARTcmd[3] = 0x04;
                break;
            case MP3_VOL_DOWN:
                USARTcmd[3] = 0x05;
                break;
            case MP3_SLEEP_MODE:
                USARTcmd[3] = 0x0A;
                break;
            case MP3_WAKE_UP:
                USARTcmd[3] = 0x0B;
                break;
            case MP3_RESET:
                USARTcmd[3] = 0x0C;
                break;
            case MP3_PLAY:
                USARTcmd[3] = 0x0D;
                break;
            case MP3_PAUSE:
                USARTcmd[3] = 0x0E;
                break;
            case MP3_STOP_PLAY:
                USARTcmd[3] = 0x16;
                break;
            case MP3_START_CYCLE:
                USARTcmd[3] = 0x19;
                USARTcmd[6] = 0x00;
                break;
            case MP3_STOP_CYCLE:
                USARTcmd[3] = 0x19;
                USARTcmd[6] = 0x01;
                break;
            case MP3_ENA_DAC:
                USARTcmd[3] = 0x1A;
                USARTcmd[6] = 0x00;
                break;
            case MP3_DIS_DAC:
                USARTcmd[3] = 0x1A;
                USARTcmd[6] = 0x01;
                break;
            default:
                USARTcmd[3] = 0x0F;
                USARTcmd[5] = 0x01;
                USARTcmd[6] = (uint8_t)(msg & 0x00FF);
                break;
        }
        USARTdrv->Send(USARTcmd, USART_LEN);
        osThreadFlagsWait(THREAD_FLAG_USART_DONE, osFlagsWaitAny, osWaitForever);
        USARTcmd[3] = 0;
        USARTcmd[5] = 0;
        USARTcmd[6] = 0;
    }
}

static void USART_Callback(uint32_t event) {
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) {
        osThreadFlagsSet(MP3_Thread, THREAD_FLAG_USART_DONE);
    }
    if (event & ARM_USART_EVENT_TX_COMPLETE) {
        osThreadFlagsSet(MP3_Thread, THREAD_FLAG_USART_DONE);
    }
}

static void USART_Init(ARM_DRIVER_USART* drv) {
    drv->Initialize(USART_Callback);
    drv->PowerControl(ARM_POWER_FULL);
    drv->Control(
        ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE | ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE,
        9600);
    drv->Control(ARM_USART_CONTROL_TX, 1);
    drv->Control(ARM_USART_CONTROL_RX, 1);
}
