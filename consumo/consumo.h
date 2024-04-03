#ifndef CONSUMO_H
#define CONSUMO_H

#include <stdint.h>
#include <cmsis_os2.h>

#define CONSUMO_MASCARA    0x0FF0
#define CONSUMO_IMPEDANCIA 110
#define CONSUMO_TO_AMP(c) (double)((3.3*(c)/4096.0)/110.0)

osThreadId_t       consumo_tid;
osMessageQueueId_t consumoToMain;

typedef uint16_t consumo_t;

int Init_Consumo(void);

#endif