#include "dspThread.h"

#include "../adc/adc.h"
#include "../dac/dac.h"
#include "../main.h"
#include "audioConfig.h"
#include "dsp.h"

#define HALF_FLAG 0x01
#define FULL_FLAG 0x02

static osThreadId_t dsp_tid;
static uint16_t     inBuffer[2 * DSP_BUFSIZE] = {0};
static uint16_t     outBuffer[2 * DSP_BUFSIZE] = {0};

static void DSP_Thread(void* arg);
static void halfBufferCb(void);
static void fullBufferCb(void);

osMessageQueueId_t dspQueue;

int DSP_Init(void) {
    dsp_tid = osThreadNew((osThreadFunc_t)DSP_Thread, NULL, NULL);
    if (dsp_tid == NULL) {
        return -1;
    }

    dspQueue = osMessageQueueNew(DSP_MSG_CNT, sizeof(dspMsg_t), NULL);
    if (dspQueue == NULL) {
        return -1;
    }

    if (sampling_init((sampling_Callback)halfBufferCb, (sampling_Callback)fullBufferCb) != 0) {
        return -1;
    }

    if (sampling_start(inBuffer, 2 * DSP_BUFSIZE) != 0) {
        return -1;
    }

    if (audio_init() != 0) {
        return -1;
    }

    if (audio_start(outBuffer, 2 * DSP_BUFSIZE) != 0) {
        return -1;
    }

    if (dsp_init() != 0) {
        return -1;
    }

    return 0;
}

void halfBufferCb(void) { osThreadFlagsSet(dsp_tid, HALF_FLAG); }

void fullBufferCb(void) { osThreadFlagsSet(dsp_tid, FULL_FLAG); }

static void DSP_Thread(void* arg) {
    uint16_t* in;
    uint16_t* out;
    dspMsg_t  msg;

    while (1) {
        int flag = osThreadFlagsWait(HALF_FLAG | FULL_FLAG, osFlagsWaitAny, osWaitForever);
        if (flag == HALF_FLAG) {
            in = inBuffer;
            out = outBuffer;
        } else {
            in = &inBuffer[DSP_BUFSIZE];
            out = &outBuffer[DSP_BUFSIZE];
        }
        processSamples(in, out);

        while (osMessageQueueGet(dspQueue, &msg, NULL, 0) != osErrorResource) {
            dsp_configure_filters(msg.bandGains, msg.vol);
        }
    }
}
