#include "dac.h"

#include "../main.h"
#include "../DSP/audioConfig.h"

static DAC_HandleTypeDef hdac = {0};
static DMA_HandleTypeDef hdma = {0};

static int dac_init(void);
static int dac_dma_init(void);

int audio_init() { return dac_init() | dac_dma_init(); }

static int dac_init(void) {
    __AUDIO_GPIO_ENABLE();
    GPIO_InitTypeDef gpioDAC
        = {.Pin = AUDIO_GPIO_PIN, .Mode = GPIO_MODE_ANALOG, .Pull = GPIO_NOPULL};
    HAL_GPIO_Init(AUDIO_GPIO_PORT, &gpioDAC);

    __AUDIO_DAC_ENABLE();
    hdac.Instance = AUDIO_DAC;
    if (HAL_DAC_Init(&hdac) != 0) {
        return -1;
    }

    DAC_ChannelConfTypeDef dacChannel
        = {.DAC_Trigger = AUDIO_DAC_TRIGGER,
           .DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE};

    if (HAL_DAC_ConfigChannel(&hdac, &dacChannel, AUDIO_DAC_CHANNEL) != 0) {
        return -1;
    }

    return 0;
}

static int dac_dma_init() {
    /* DMA controller clock enable */
    __AUDIO_DMA_ENABLE();

    hdma.Instance                 = AUDIO_DMA_INSTANCE;
    hdma.Init.Channel             = AUDIO_DMA_CHANNEL;
    hdma.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma.Init.MemInc              = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    hdma.Init.Mode                = DMA_CIRCULAR;
    hdma.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    hdma.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma) != HAL_OK) {
        return -1;
    }

    __HAL_LINKDMA(&hdac, AUDIO_DMA_HANDLE, hdma);

    return 0;
}

int audio_start(uint16_t* buffer, uint32_t bufSize) {
    return HAL_DAC_Start_DMA(&hdac, AUDIO_DAC_CHANNEL, (uint32_t*)buffer, bufSize,
                             DAC_ALIGN_12B_R);
}

void audio_stop(void) { HAL_DAC_Stop_DMA(&hdac, AUDIO_DAC_CHANNEL); }
