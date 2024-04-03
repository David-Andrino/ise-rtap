#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>
#include <cmsis_os2.h>

typedef enum {
    POWERON     = 0,
    POWEROFF    = 1,
    SEEKUP      = 2,
    SEEKDOWN    = 3,
    INFO        = 4,
    // Resto de valores, sintonizar
} radioMsg_t;

typedef struct {
    uint32_t freq;
    uint32_t power;
} radioResponse_t;

osThreadId_t radio_tid;
osMessageQueueId_t radioToMainQueue, mainToRadioQueue;

int Init_Radio(void);

#endif