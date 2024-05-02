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

// BORRAR: TEST
static TIM_HandleTypeDef htmptim = {0};
static dspMsg_t testMsg = { .vol = 10 };

static int tmp_tim_init(void) {
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_TIM4_CLK_ENABLE();
    GPIO_InitTypeDef tmpTimGPIO = {
                      .Pin = GPIO_PIN_12,
                      .Mode = GPIO_MODE_AF_PP,
                      .Alternate = GPIO_AF2_TIM4,
                      .Pull = GPIO_NOPULL,
    };
    HAL_GPIO_Init(GPIOD, &tmpTimGPIO);

    htmptim.Instance = TIM4;
    htmptim.Init.Prescaler = 1999;
    htmptim.Init.Period = 209;
    if (HAL_TIM_OC_Init(&htmptim)) {
        return -1;
    }

    TIM_OC_InitTypeDef tmpTimChannel = {
                      .OCMode = TIM_OCMODE_TOGGLE,
                      .OCPolarity = TIM_OCPOLARITY_HIGH,
                      .OCFastMode = TIM_OCFAST_DISABLE,
    };

    if (HAL_TIM_OC_ConfigChannel(&htmptim, &tmpTimChannel, TIM_CHANNEL_1)) {
        return -1;
    }

    if (HAL_TIM_OC_Start(&htmptim, TIM_CHANNEL_1)) {
        return -1;
    }

    return 0;
}
void EXTI15_10_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_13) {
        testMsg.vol = (testMsg.vol == 0) ? 10 : testMsg.vol - 1;
        osMessageQueuePut(dspQueue, &testMsg, NULL, 0);
    }
}

// FIN TEST

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

    // BORRAR: TEST
    tmp_tim_init();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef pc13 = {
        .Pin = GPIO_PIN_13,
        .Mode = GPIO_MODE_IT_FALLING,
        .Pull = GPIO_NOPULL
    };
    HAL_GPIO_Init(GPIOC, &pc13);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    // FIN TEST

    return 0;
}

void halfBufferCb(void) { osThreadFlagsSet(dsp_tid, HALF_FLAG); }

void fullBufferCb(void) { osThreadFlagsSet(dsp_tid, FULL_FLAG); }

static void DSP_Thread(void* arg) {
    uint16_t* in;
    uint16_t* out;
    dspMsg_t msg;
    
    while (1) {
        int flag = osThreadFlagsWait(HALF_FLAG | FULL_FLAG, osFlagsWaitAny, osWaitForever);
        if (flag == HALF_FLAG) {
            in  = inBuffer;
            out = outBuffer;
        } else {
            in = &inBuffer[DSP_BUFSIZE];
            out = &outBuffer[DSP_BUFSIZE];
        }
        processSamples(in, out);
        
        if (osMessageQueueGetCount(dspQueue) > 0) {
            while (osMessageQueueGet(dspQueue, &msg, NULL, 0) != osErrorResource);
            dsp_configure_filters(msg.bandGains, msg.vol);
        }
    }
}
