#ifndef NFC_H
#define NFC_H

#include <stdint.h>
#include <cmsis_os2.h>

#define NFC_FLAG 0x01

extern osThreadId_t nfc_tid;

int Init_NFC(void);

#endif
