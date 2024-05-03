#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>
#include <cmsis_os2.h>
#include "controlThread.h"

#define    POWERON 0
#define    POWEROFF   1
#define    SEEKUP       2
#define    SEEKDOWN     3
#define    INFO         4
    // Resto de valores, sintonizar
typedef uint32_t radioMsg_t;


extern osThreadId_t radio_tid;
extern osMessageQueueId_t radioToMainQueue, mainToRadioQueue;

extern int Init_Radio(void);

#endif