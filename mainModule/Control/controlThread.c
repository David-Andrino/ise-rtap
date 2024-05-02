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

static osThreadId_t  ctrl_tid;
static radioMsg_t    radioMsg;
static mp3Msg_t      mp3msg;
static dspMsg_t      dspMsg = { .vol = 10, .bandGains = { 0 } };
static lcd_out_msg_t lcdMsg;
static web_state_t   webMsg;
static uint8_t mp3Playing = 0;

static void Control_Thread(void* arg);
static void GPIO_init(void);
static void ctrl_LCD(lcd_msg_t* msg);
static void ctrl_RTC(rtc_msg_t* msg);
static void ctrl_NFC(nfc_msg_t* msg);
static void ctrl_WEB(web_msg_t* msg);
static void ctrl_lowPower(void);
static void ctrl_saveConfig(void);

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
    switch (msg->type) {
        case LCD_INPUT_SEL:
            if (msg->payload == 0) { // Seleccionar radio
                HAL_GPIO_WritePin(INPUT_SELECT_GPIO_PORT, INPUT_SELECT_GPIO_PIN, SELECT_INPUT_RADIO);
            } else {                 // Seleccionar MP3
                HAL_GPIO_WritePin(INPUT_SELECT_GPIO_PORT, INPUT_SELECT_GPIO_PIN, SELECT_INPUT_MP3);
            }

            webMsg.type = WEB_OUT_INPUT_SEL;
            webMsg.payload = msg->payload;
            osMessageQueuePut(webQueue, &webMsg, NULL, 0);
            break;

        case LCD_OUTPUT_SEL:
            if (msg->payload == 0) { // Seleccionar cascos
                HAL_GPIO_WritePin(OUTPUT_SELECT_GPIO_PORT, OUTPUT_SELECT_GPIO_PIN, SELECT_OUTPUT_EAR);
            } else {                 // Seleccionar altavoz
                HAL_GPIO_WritePin(OUTPUT_SELECT_GPIO_PORT, OUTPUT_SELECT_GPIO_PIN, SELECT_OUTPUT_SPK);
            }

            webMsg.type = web_OUT_OUTPUT_SEL;
            webMsg.payload = msg->payload;
            osMessageQueuePut(webQueue, &webMsg, NULL, 0);
            break;

        case LCD_VOL:
            if (msg->payload >= 0 && msg->payload <= 10) {
                dspMsg.vol = msg->payload;
                osMessageQueuePut(dspQueue, &dspMsg, NULL, 0);

                webMsg.type = WEB_OUT_VOL;
                webMsg.payload = msg->payload;
                osMessageQueuePut(webQueue, &webMsg, NULL, 0);
            }
            break;

        case LCD_BANDS:
            if (((msg->payload >> 8) < 5) && ((msg->payload >> 8) >= 0)) {
                dspMsg.bandGains[msg->payload >> 8] = msg->payload & 0xFF;
                osMessageQueuePut(dspQueue, &dspMsg, NULL, 0);

                webMsg.type = WEB_OUT_BANDS;
                webMsg.payload = msg->payload;
                osMessageQueuePut(webQueue, &webMsg, NULL, 0);
            }
            break;

        case LCD_RADIO_FREQ:
            if (msg->payload >= RADIO_MIN_FREQ && msg->payload <= RADIO_MAX_FREQ) {
                radioMsg = msg->payload * 100;
                osMessageQueuePut(mainToRadioQueue, &radioMsg, NULL, 0);

                webMsg.type = WEB_OUT_RADIO_FREQ;
                webMsg.payload = msg->payload;
                osMessageQueuePut(webQueue, &webMsg, NULL, 0);
            }
            break;

        case LCD_SONG:
            mp3msg = msg->content;
            osMessageQueuePut(MP3Queue, &mp3msg, NULL, 0);
            break;

        case LCD_SAVE_SD:
            ctrl_saveConfig();
            break;

        case LCD_LOW_POWER:
            ctrl_lowPower();
            break;

        case LCD_LOOP:
            mp3msg = MP3_START_CYCLE;
            osMessageQueuePut(MP3Queue, &mp3msg, NULL, 0);
            break;

        case LCD_SEEK:
            radioMsg = (msg->payload == 0) ? SEEKDOWN : SEEKUP;
            osMessageQueuePut(mainToRadioQueue, &radioMsg, NULL, 0);
            break;

        case LCD_NEXT_SONG:
            mp3msg = MP3_NEXT_SONG;
            osMessageQueuePut(MP3Queue, &mp3msg, NULL, 0);
            break;

        case LCD_PREV_SONG:
            mp3msg = MP3_PREV_SONG;
            osMessageQueuePut(MP3Queue, &mp3msg, NULL, 0);
            break;

        case LCD_PLAY_PAUSE:
            mp3msg = mp3Playing ? MP3_PAUSE : MP3_PLAY;
            mp3Playing = !mp3Playing;
            osMessageQueuePut(MP3Queue, &mp3msg, NULL, 0);
            break;
    }
}

static void ctrl_RTC(rtc_msg_t* msg) {
    webMsg.type    = WEB_OUT_DATE;
    webMsg.payload = (msg->day << 16) | (msg->month << 8) | msg->year;
    osMessageQueuePut(webQueue, &webMsg, NULL, 0);

    webMsg.type    = WEB_OUT_HOUR;
    webMsg.payload = (msg->hour << 16) | (msg->minute << 8) | msg->second;
    osMessageQueuePut(webQueue, &webMsg, NULL, 0);
}

static void ctrl_NFC(nfc_msg_t* msg) {
    if (msg->type == 0) { // Cancion
        mp3msg = msg->content;
        osMessageQueuePut(MP3Queue, &msg->content, NULL, 0);
    } else {              // Radio
        radioMsg = 100 * msg->content;
        osMessageQueuePut(mainToRadioQueue, &radioMsg, NULL, 0);
    }
}

// ========================================================
// TODOs de controlador de web
// TODO: Guardar datos en SD. Bucle infinito para acordarme
// TODO: La radio tiene que devolver en que frecuencia ha acabado
// ========================================================

static void ctrl_WEB(web_msg_t* msg) {
    switch (msg->type) {
        case WEB_INPUT_SEL:
            if (msg->payload == 0) { // Seleccionar radio
                HAL_GPIO_WritePin(INPUT_SELECT_GPIO_PORT, INPUT_SELECT_GPIO_PIN, SELECT_INPUT_RADIO);
            } else {                 // Seleccionar MP3
                HAL_GPIO_WritePin(INPUT_SELECT_GPIO_PORT, INPUT_SELECT_GPIO_PIN, SELECT_INPUT_MP3);
            }

            lcdMsg.type = LCD_OUT_INPUT_SEL;
            lcdMsg.payload = msg->payload;
            osMessageQueuePut(lcdQueue, &lcdMsg, NULL, 0);
            break;

        case WEB_OUTPUT_SEL:
            if (msg->payload == 0) { // Seleccionar cascos
                HAL_GPIO_WritePin(OUTPUT_SELECT_GPIO_PORT, OUTPUT_SELECT_GPIO_PIN, SELECT_OUTPUT_EAR);
            } else {                 // Seleccionar altavoz
                HAL_GPIO_WritePin(OUTPUT_SELECT_GPIO_PORT, OUTPUT_SELECT_GPIO_PIN, SELECT_OUTPUT_SPK);
            }

            lcdMsg.type = LCD_OUT_OUTPUT_SEL;
            lcdMsg.payload = msg->payload;
            osMessageQueuePut(lcdQueue, &lcdMsg, NULL, 0);
            break;

        case WEB_LOW_POWER:
            ctrl_lowPower();
            break;

        case WEB_RADIO_FREQ:
            if (msg->payload >= RADIO_MIN_FREQ && msg->payload <= RADIO_MAX_FREQ) {
                radioMsg = msg->payload * 100;
                osMessageQueuePut(mainToRadioQueue, &radioMsg, NULL, 0);

                lcdMsg.type = LCD_OUT_RADIO_FREQ;
                lcdMsg.payload = msg->payload;
                osMessageQueuePut(lcdQueue, &lcdMsg, NULL, 0);
            }
            break;

        case WEB_SEEK:
            radioMsg = (msg->payload == 0) ? SEEKDOWN : SEEKUP;
            osMessageQueuePut(mainToRadioQueue, &radioMsg, NULL, 0);
            break;

        case WEB_VOL:
            if (msg->payload >= 0 && msg->payload <= 10) {
                dspMsg.vol = msg->payload;
                osMessageQueuePut(dspQueue, &dspMsg, NULL, 0);

                lcdMsg.type = LCD_OUT_VOL;
                lcdMsg.payload = msg->payload;
                osMessageQueuePut(lcdQueue, &lcdMsg, NULL, 0);
            }
            break;

        case WEB_SONG:
            mp3msg = msg->content;
            osMessageQueuePut(MP3Queue, &mp3msg, NULL, 0);
            break;

        case WEB_PLAY_PAUSE:
            mp3msg = mp3Playing ? MP3_PAUSE : MP3_PLAY;
            mp3Playing = !mp3Playing;
            osMessageQueuePut(MP3Queue, &mp3msg, NULL, 0);
            break;

        case WEB_PREV_SONG:
            mp3msg = MP3_PREV_SONG;
            osMessageQueuePut(MP3Queue, &mp3msg, NULL, 0);
            break;

        case WEB_NEXT_SONG:
            mp3msg = MP3_NEXT_SONG;
            osMessageQueuePut(MP3Queue, &mp3msg, NULL, 0);
            break;

        case WEB_BANDS:
            if (((msg->payload >> 8) < 5) && ((msg->payload >> 8) >= 0)) {
                dspMsg.bandGains[msg->payload >> 8] = msg->payload & 0xFF;
                osMessageQueuePut(dspQueue, &dspMsg, NULL, 0);
                lcdMsg.type = LCD_OUT_BANDS;
                lcdMsg.payload = msg->payload;
                osMessageQueuePut(lcdQueue, &lcdMsg, NULL, 0);
            }
            break;

        case WEB_SAVE_SD:
            ctrl_saveConfig();
            break;

        case WEB_LOOP:
            mp3msg = MP3_START_CYCLE;
            osMessageQueuePut(MP3Queue, &mp3msg, NULL, 0);
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

// TODO: Low power mode
void ctrl_lowPower(void) {
    while (1) {}
}

// TODO: Save to SD
void ctrl_saveConfig() {
    while (1) {}
}