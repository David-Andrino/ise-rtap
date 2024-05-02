#include "controlThread.h"
#include "../main.h"
#include "../MP3/mp3.h"
#include "../Radio/radio.h"
#include "../WEB/ThreadWeb.h"
#include "../DSP/dspThread.h"
#include "controlConfig.h"
#include <cmsis_os2.h>

#define QUEUE_SIZE 64

osMessageQueueId_t ctrl_in_queue;

static osThreadId_t ctrl_tid;
static radioMsg_t   radioMsg;
static mp3Msg_t     mp3msg;

static void Control_Thread(void* arg);
static void GPIO_init(void);
static void ctrl_LCD(lcd_msg_t* msg);
static void ctrl_RTC(rtc_msg_t* msg);
static void ctrl_NFC(nfc_msg_t* msg);
static void ctrl_WEB(web_msg_t* msg);
static void ctrl_lowPower(void);

int Init_Control(void) {
    GPIO_init();
    
    ctrl_tid = osThreadNew(Control_Thread, NULL, NULL);
    if (ctrl_tid == NULL) {
        return -1;
    }
    
    ctrl_in_queue = osMessageQueueNew(QUEUE_SIZE, sizeof(ctrl_in_queue), NULL);
    if (ctrl_in_queue == NULL) {
        return -1;
    }
    
    return 0;
}

static void Control_Thread(void* arg) {
    msg_ctrl_t msg;
    while (1) {
        osMessageQueueGet(ctrl_in_queue, &msg, NULL, osWaitForever);
        switch (msg.type) {
            case MSG_LCD:
                ctrl_LCD(&msg.lcd_msg);
                break;
            case MSG_NFC:
                ctrl_NFC(&msg.nfc_msg);
                break;
            case MSG_RTC:
                ctrl_RTC(&msg.rtc_msg);
                break;
            default: case MSG_WEB:
                ctrl_WEB(&msg.web_msg);
                break;
        }
    }
}

static void ctrl_LCD(lcd_msg_t* msg) {
    
}

static void ctrl_RTC(rtc_msg_t* msg) {
    // Send new hour to LCD and WEB
}

// El NFC solo puede cambiar la emisora y la cancion
static void ctrl_NFC(nfc_msg_t* msg) {
    if (msg->type == 0) { // Cancion
        mp3msg = msg->content;
        osMessageQueuePut(MP3Queue, &mp3msg, NULL, 0);
    } else {              // Radio
        radioMsg = 100 * msg->content;
        osMessageQueuePut(mainToRadioQueue, &radioMsg, NULL, 0);
    }
}

static void ctrl_WEB(web_msg_t* msg) {
    switch (msg->type) {
        case WEB_INPUT_SEL:
            if (msg->payload == 0) { // Seleccionar radio
                HAL_GPIO_WritePin(INPUT_SELECT_GPIO_PORT, INPUT_SELECT_GPIO_PIN, SELECT_INPUT_RADIO);
            } else {                 // Seleccionar MP3
                HAL_GPIO_WritePin(INPUT_SELECT_GPIO_PORT, INPUT_SELECT_GPIO_PIN, SELECT_INPUT_MP3);
            }
            break;
        case WEB_OUTPUT_SEL:
            if (msg->payload == 0) { // Seleccionar cascos
                HAL_GPIO_WritePin(OUTPUT_SELECT_GPIO_PORT, OUTPUT_SELECT_GPIO_PIN, SELECT_OUTPUT_EAR);
            } else {                 // Seleccionar altavoz
                HAL_GPIO_WritePin(OUTPUT_SELECT_GPIO_PORT, OUTPUT_SELECT_GPIO_PIN, SELECT_OUTPUT_SPK);
            }
            break;
        case WEB_LOW_POWER:
            ctrl_lowPower();
            break;
        case WEB_RADIO_FREQ:
            if (msg->payload >= 870 && msg->payload <= 1080) {
                radioMsg = msg->payload * 100;
                osMessageQueuePut(mainToRadioQueue, &radioMsg, NULL, 0);
            }
            break;
        case WEB_SEEK:
            radioMsg = (msg->payload == 0) ? SEEKDOWN : SEEKUP;
            osMessageQueuePut(mainToRadioQueue, &radioMsg, NULL, 0);
            break;
        case WEB_VOL:
            
            break;
        case WEB_SONG:
            break;
        case WEB_PLAY_PAUSE:
            break;
        case WEB_PREV_SONG:
            break;
        case WEB_NEXT_SONG:
            break;
        case WEB_BANDS:
            break;
        case WEB_SAVE_SD:
            break;
        case WEB_LOOP:
            break;
    }
}

void GPIO_init(void) {
    __INPUT_SELECT_GPIO();
    GPIO_InitTypeDef radioGPIO = {
        .Pin = INPUT_SELECT_GPIO_PIN,
        .Pull = GPIO_NOPULL, 
        .Mode = GPIO_MODE_OUTPUT_PP,
    };
    HAL_GPIO_Init(INPUT_SELECT_GPIO_PORT, &radioGPIO);
    
    __OUTPUT_SELECT_GPIO();
    GPIO_InitTypeDef mp3GPIO = {
        .Pin = OUTPUT_SELECT_GPIO_PIN,
        .Pull = GPIO_NOPULL, 
        .Mode = GPIO_MODE_OUTPUT_PP,
    };
    HAL_GPIO_Init(OUTPUT_SELECT_GPIO_PORT, &mp3GPIO);
    
    __CONS_ENABLE_GPIO();
    GPIO_InitTypeDef consGPIO = {
        .Pin = CONS_GPIO_PIN,
        .Pull = GPIO_NOPULL, 
        .Mode = GPIO_MODE_ANALOG,
    };
    HAL_GPIO_Init(CONS_GPIO_PORT, &consGPIO);
}

void ctrl_lowPower(void) {}