#ifndef MP3_H
#define MP3_H

#include "cmsis_os2.h"
#include "Driver_USART.h"
#include "../main.h"

typedef enum {
		MP3_NEXT_SONG = 256,
		MP3_PREV_SONG,
		MP3_VOL_UP,
		MP3_VOL_DOWN,
		MP3_SLEEP_MODE,
		MP3_WAKE_UP,
		MP3_RESET,
		MP3_PLAY,
		MP3_PAUSE,
		MP3_STOP_PLAY,
		MP3_START_CYCLE,
		MP3_STOP_CYCLE,
		MP3_ENA_DAC,
		MP3_DIS_DAC
} mp3Msg_t;

extern osThreadId_t MP3_Thread;
extern osMessageQueueId_t MP3Queue;

extern int Init_MP3(void);

#endif
