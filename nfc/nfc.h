#ifndef NFC_H
#define NFC_H

#include <stdint.h>
#include <cmsis_os2.h>
#include "Driver_I2C.h"
#include "stm32f4xx_hal.h"

#define NFC_FLAG 0x01

typedef struct {
  uint8_t modo;
  uint8_t id;
} nfcReading_t;

extern osThreadId_t nfc_tid;
extern osMessageQueueId_t nfcToMainQueue;

extern int Init_NFC(void);

int I2C_Init(void);
void GPIO_Init(void);
void Leer_NFC(void);
void OpenSession(void);
void SelectNFC(void);
void SelectFileSystem(void);
void ReadFileSystem(void);
void WriteFileSystem(void);
void SelectCC(void);
void ReadCCLength(void);
void ReadCCFile(void);
void SelectNDEF(void);
void ReadNDEFLength(void);
void ReadNDEF(void);
void Deselect(void);
void Verify(void);

#endif
