#ifndef THREADSINT_H
#define THREADSINT_H

#include "cmsis_os2.h" 
#include "main.h"
#include "Driver_I2C.h"

extern ARM_DRIVER_I2C Driver_I2C2;
static ARM_DRIVER_I2C* I2Cdrv2 = &Driver_I2C2;

static uint16_t RDA5807M_WriteReg[6];
static uint16_t RDA5807M_ReadReg[6];

extern osThreadId_t tid_Sintonizador;

extern int Init_Sintonizador (void);
extern osMessageQueueId_t msgQueue;
extern void SeekUp(void);
extern uint32_t seeFrec(void);

typedef struct {
	uint32_t freq;
	uint8_t vol;
} msg_t;

#define RDA_TUNE_ON     0x0010
#define RDA_TUNE_OFF    0xFFEF
#define RDA_POWER       0x0001
#define RDA_RDS_ON      0x0008
#define RDA_SEEK_UP 		0x0300
#define RDA_SEEK_STOP		0xFCFF

#define StartingFreq    87.00
#define InitialVolume       15

#define MAX_MSG 10

#define RDA_TRANSFER_COMPLETE 0x20

#endif
