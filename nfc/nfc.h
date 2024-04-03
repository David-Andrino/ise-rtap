#ifndef NFC_H
#define NFC_H

#include <stdint.h>
#include <cmsis_os2.h>

#define NFC_SIZE_BYTES 512

typedef enum { READ_CARD, WRITE_CARD } nfcMsgType_t;
typedef uint8_t nfcCardContent_t[NFC_SIZE_BYTES];

typedef struct {
    nfcMsgType_t type;
    nfcCardContent_t content;
} nfcMsg_t;

osThreadId_t nfc_tid;
osMessageQueueId_t nfcToMainQueue, mainToNfcQueue;

int Init_NFC(void);

#endif