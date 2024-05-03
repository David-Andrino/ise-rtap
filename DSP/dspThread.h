#ifndef DSP_THREAD_H
#define DSP_THREAD_H

#include <cmsis_os2.h>

#define DSP_MSG_CNT 16

typedef struct {
    uint8_t vol;
    int8_t  bandGains[5];
} dspMsg_t;

extern osMessageQueueId_t dspQueue;

extern int DSP_Init(void);

#endif
