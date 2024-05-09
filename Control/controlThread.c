#include "controlThread.h"

#include <cmsis_os2.h>

#include "../DSP/dspThread.h"
#include "../LCD/gui_threads.h"
#include "../MP3/mp3.h"
#include "../Radio/radio.h"
#include "../WEB/ThreadWeb.h"
#include "../main.h"
#include "controlConfig.h"
#include "../SD/sd.h"

#define QUEUE_SIZE 64

osMessageQueueId_t ctrl_in_queue;
ADC_HandleTypeDef  hconsadc;

static osTimerId_t   cons_tim;
static osThreadId_t  ctrl_tid, cons_tid;
static radioMsg_t    radioMsg;
static mp3Msg_t      mp3msg;
static dspMsg_t      dspMsg = {.vol = 10, .bandGains = {0}};
static lcd_out_msg_t lcdMsg;
static web_out_msg_t webMsg;
static uint8_t       mp3Playing = 0;
static uint32_t      exec1;  // Para el timer, ignorar
static sd_config_t   sd_config = { .volume = 10 };

static void Control_Thread(void* arg);
static void Cons_Thread(void* arg);
static void ctrl_pins_init(void);
static void ctrl_cons_init(void);
static void ctrl_LCD(lcd_msg_t* msg);
static void ctrl_RTC(rtc_msg_t* msg);
static void ctrl_NFC(nfc_msg_t* msg);
static void ctrl_WEB(web_msg_t* msg);
static void ctrl_radio(uint32_t* msg);
static void ctrl_cons(uint16_t* msg);
static void ctrl_lowPower(void);
static void ctrl_saveConfig(void);

int Init_Control(sd_config_t* initial_config) {
    ctrl_pins_init();
    ctrl_cons_init();

    ctrl_tid = osThreadNew(Control_Thread, NULL, NULL);
    if (ctrl_tid == NULL) {
        return -1;
    }

    osMessageQueueAttr_t attrs = { .name = "CtrlInQueue" };
    ctrl_in_queue = osMessageQueueNew(QUEUE_SIZE, sizeof(msg_ctrl_t), &attrs);
    if (ctrl_in_queue == NULL) {
        return -1;
    }

    cons_tid = osThreadNew(Cons_Thread, NULL, NULL);
    if (cons_tid == NULL) {
        return -1;
    }
    
    for (int i = 0; i < 5; i++) sd_config.bands[i] = initial_config->bands[i];
    sd_config.volume = initial_config->volume;

    return 0;
}

static void Control_Thread(void* arg) {
    msg_ctrl_t msg;
    
    if (sd_config.bands[0] != 0 || sd_config.bands[1] != 0 || sd_config.bands[2] != 0
        || sd_config.bands[3] != 0 || sd_config.bands[4] != 0 || sd_config.volume != 10) {

        msg.type = MSG_LCD;
        msg.lcd_msg.type = LCD_BANDS;
        for (int i = 0; i < 5; i++) {
            msg.lcd_msg.payload = (i << 8) | (sd_config.bands[i] & 0xFF);
            osMessageQueuePut(ctrl_in_queue, &msg, NULL, 0);
            osDelay(1);
        }
        
        msg.lcd_msg.type = LCD_VOL;
        msg.lcd_msg.payload = sd_config.volume;
        osMessageQueuePut(ctrl_in_queue, &msg, NULL, 0);
    }
    // Encender MP3
    mp3msg = MP3_WAKE_UP;
    osMessageQueuePut(MP3Queue, &mp3msg, NULL, osWaitForever);
    
    // Encender Radio
    radioMsg = POWERON;
    osMessageQueuePut(mainToRadioQueue, &radioMsg, NULL, osWaitForever);
    
    // Encender amplificador de audio
    HAL_GPIO_WritePin(ENA_GPIO_PORT, ENA_GPIO_PIN, ENA_GPIO_ON);
    
    osTimerStart(cons_tim, 1000);
    
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
            case MSG_RADIO:
                ctrl_radio(&msg.radio_msg);
                break;
            case MSG_CONS:
                ctrl_cons(&msg.cons_msg);
                break;
            case MSG_WEB:
                ctrl_WEB(&msg.web_msg);
                break;
            default:
                break;
        }
    }
}

static void ctrl_LCD(lcd_msg_t* msg) {
    switch (msg->type) {
        case LCD_INPUT_SEL:
            if (msg->payload == 0) {  // Seleccionar radio
                HAL_GPIO_WritePin(INPUT_SELECT_GPIO_PORT, INPUT_SELECT_GPIO_PIN, SELECT_INPUT_RADIO);
            } else {  // Seleccionar MP3
                HAL_GPIO_WritePin(INPUT_SELECT_GPIO_PORT, INPUT_SELECT_GPIO_PIN, SELECT_INPUT_MP3);
            }

            webMsg.type = WEB_OUT_INPUT_SEL;
            webMsg.payload = msg->payload;
            osMessageQueuePut(webQueue, &webMsg, NULL, 0);
            break;

        case LCD_OUTPUT_SEL:
            if (msg->payload == 0) {  // Seleccionar cascos
                HAL_GPIO_WritePin(OUTPUT_SELECT_GPIO_PORT, OUTPUT_SELECT_GPIO_PIN, SELECT_OUTPUT_EAR);
            } else {  // Seleccionar altavoz
                HAL_GPIO_WritePin(OUTPUT_SELECT_GPIO_PORT, OUTPUT_SELECT_GPIO_PIN, SELECT_OUTPUT_SPK);
            }

            webMsg.type = WEB_OUT_OUTPUT_SEL;
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
            mp3msg = msg->payload;
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
    // Update WEB date and time
    webMsg.type = WEB_OUT_DATE;
    webMsg.payload = (msg->day << 16) | (msg->month << 8) | msg->year;
    osMessageQueuePut(webQueue, &webMsg, NULL, 0);

    webMsg.type = WEB_OUT_HOUR;
    webMsg.payload = (msg->hour << 16) | (msg->minute << 8) | msg->second;
    osMessageQueuePut(webQueue, &webMsg, NULL, 0);
}
static void ctrl_NFC(nfc_msg_t* msg) {
    if (msg->type == 0) {  // Cancion
        mp3msg = msg->content;
        osMessageQueuePut(MP3Queue, &msg->content, NULL, 0);
    } else {  // Radio
        radioMsg = 100 * msg->content;
        osMessageQueuePut(mainToRadioQueue, &radioMsg, NULL, 0);
        
        // Actualizar frecuencia del LCD y WEB
        lcdMsg.type = LCD_OUT_RADIO_FREQ;
        lcdMsg.payload = msg->content;
        osMessageQueuePut(lcdQueue, &lcdMsg, NULL, 0);

        webMsg.type = WEB_OUT_RADIO_FREQ;
        webMsg.payload = lcdMsg.payload;
        osMessageQueuePut(webQueue, &webMsg, NULL, 0);
    }
}

// ========================================================
// TODOs de controlador de web
// TODO: Guardar datos en SD. Bucle infinito para acordarme
// ========================================================
static void ctrl_WEB(web_msg_t* msg) {
    switch (msg->type) {
        case WEB_INPUT_SEL:
            if (msg->payload == 0) {  // Seleccionar radio
                HAL_GPIO_WritePin(INPUT_SELECT_GPIO_PORT, INPUT_SELECT_GPIO_PIN, SELECT_INPUT_RADIO);
            } else {  // Seleccionar MP3
                HAL_GPIO_WritePin(INPUT_SELECT_GPIO_PORT, INPUT_SELECT_GPIO_PIN, SELECT_INPUT_MP3);
            }

            lcdMsg.type = LCD_OUT_INPUT_SEL;
            lcdMsg.payload = msg->payload;
            osMessageQueuePut(lcdQueue, &lcdMsg, NULL, 0);
            break;

        case WEB_OUTPUT_SEL:
            if (msg->payload == 0) {  // Seleccionar cascos
                HAL_GPIO_WritePin(OUTPUT_SELECT_GPIO_PORT, OUTPUT_SELECT_GPIO_PIN, SELECT_OUTPUT_EAR);
            } else {  // Seleccionar altavoz
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
            mp3msg = msg->payload ;
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

static void ctrl_radio(uint32_t* msg) {
    // Actualizar frecuencia del LCD y WEB
    lcdMsg.type = LCD_OUT_RADIO_FREQ;
    lcdMsg.payload = (*msg) / 100;
    osMessageQueuePut(lcdQueue, &lcdMsg, NULL, 0);

    webMsg.type = WEB_OUT_RADIO_FREQ;
    webMsg.payload = lcdMsg.payload;
    osMessageQueuePut(webQueue, &webMsg, NULL, 0);
}

static void ctrl_cons(uint16_t* msg) {
    uint16_t cons = *msg;
    webMsg.type = WEB_OUT_CONS;
    webMsg.payload = ((uint32_t)(*msg) * 3000)/4096;
    osMessageQueuePut(webQueue, &webMsg, NULL, 0);
    
    lcdMsg.type = LCD_OUT_CONS;
    lcdMsg.payload = webMsg.payload;
    osMessageQueuePut(lcdQueue, &lcdMsg, NULL, 0);
}
    
static void ctrl_pins_init(void) {
    __ENA_GPIO();
    GPIO_InitTypeDef enaGPIO = {
        .Pin = ENA_GPIO_PIN,
        .Pull = GPIO_NOPULL,
        .Mode = GPIO_MODE_OUTPUT_PP,
    };
    HAL_GPIO_Init(ENA_GPIO_PORT, &enaGPIO);

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

    HAL_GPIO_WritePin(ENA_GPIO_PORT, ENA_GPIO_PIN, ENA_GPIO_ON);
    HAL_GPIO_WritePin(INPUT_SELECT_GPIO_PORT, INPUT_SELECT_GPIO_PIN, SELECT_INPUT_RADIO);
    HAL_GPIO_WritePin(OUTPUT_SELECT_GPIO_PORT, OUTPUT_SELECT_GPIO_PIN, SELECT_OUTPUT_EAR);
}

static void ctrl_lowPower(void) {
    // Apagar MP3
    mp3msg = MP3_SLEEP_MODE;
    osMessageQueuePut(MP3Queue, &mp3msg, NULL, osWaitForever);
    
    // Apagar Radio
    radioMsg = POWEROFF;
    osMessageQueuePut(mainToRadioQueue, &radioMsg, NULL, osWaitForever);
    
    // Apagar amplificador de audio
    HAL_GPIO_WritePin(ENA_GPIO_PORT, ENA_GPIO_PIN, ENA_GPIO_OFF);
    
    osDelay(10); // Dar tiempo a que MP3 y RADIO acaben
    
    EnterStandbyMode();
}

static void ctrl_saveConfig() {
    sd_config_t config = {
        .volume = dspMsg.vol,
        .bands[0] = dspMsg.bandGains[0],
        .bands[1] = dspMsg.bandGains[1],
        .bands[2] = dspMsg.bandGains[2],
        .bands[3] = dspMsg.bandGains[3],
        .bands[4] = dspMsg.bandGains[4],
    };
    volatile int tmp = SD_write_config(&config); // No funciona :(
}

static void ctrl_cons_init(void) {
    __CONS_ENABLE_GPIO();
    GPIO_InitTypeDef consGPIO = {
        .Pin = CONS_GPIO_PIN,
        .Pull = GPIO_NOPULL,
        .Mode = GPIO_MODE_ANALOG,
    };
    HAL_GPIO_Init(CONS_GPIO_PORT, &consGPIO);

    __ENA_CONS_ADC();
    hconsadc.Instance = CONS_ADC;
    hconsadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
    hconsadc.Init.Resolution = ADC_RESOLUTION_12B;
    hconsadc.Init.ScanConvMode = ENABLE;
    hconsadc.Init.ContinuousConvMode = DISABLE;
    hconsadc.Init.DiscontinuousConvMode = DISABLE;
    hconsadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hconsadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hconsadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hconsadc.Init.NbrOfConversion = 1;
    hconsadc.Init.DMAContinuousRequests = DISABLE;
    hconsadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    HAL_ADC_Init(&hconsadc);

    ADC_ChannelConfTypeDef sConfig = {
        .Channel = CONS_ADC_CHANNEL,
        .Rank = 1,
        .SamplingTime = ADC_SAMPLETIME_480CYCLES};
    HAL_ADC_ConfigChannel(&hconsadc, &sConfig);
    HAL_ADC_Start(&hconsadc);
}

static void Cons_Thread(void* arg) {
    msg_ctrl_t msg = {.type = MSG_CONS};
    
    while (1) {
        HAL_ADC_Start(&hconsadc);
        osDelay(1000);
        HAL_ADC_Stop(&hconsadc);
        msg.cons_msg = HAL_ADC_GetValue(&hconsadc);
        osMessageQueuePut(ctrl_in_queue, &msg, NULL, osWaitForever);
    }
}
