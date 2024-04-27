#include "dac.h"

#include "../main.h"

static TIM_HandleTypeDef htim = {0};
static DAC_HandleTypeDef hdac = {0};
static DMA_HandleTypeDef hdma = {0};

static int dac_init(void);
static int dac_tim_init(void);
static int dac_dma_init(void);

int audio_init() { return dac_tim_init() | dac_init() | dac_dma_init(); }

static int dac_init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef gpioPA4
        = {.Pin = GPIO_PIN_4, .Mode = GPIO_MODE_ANALOG, .Pull = GPIO_NOPULL};
    HAL_GPIO_Init(GPIOA, &gpioPA4);

    __HAL_RCC_DAC_CLK_ENABLE();
    hdac.Instance = DAC1;
    HAL_DAC_Init(&hdac);

    DAC_ChannelConfTypeDef dacChannel
        = {.DAC_Trigger = DAC_TRIGGER_T2_TRGO,
           .DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE};

    HAL_DAC_ConfigChannel(&hdac, &dacChannel, DAC_CHANNEL_1);

    return 0;
}

int dac_tim_init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();
    htim.Instance = TIM2;

    htim.Init.Prescaler = 9;
    htim.Init.Period = 174;

    if (HAL_TIM_Base_Init(&htim)) {
        return -1;
    }

    TIM_ClockConfigTypeDef sClockConfig
        = {.ClockSource = TIM_CLOCKSOURCE_INTERNAL};
    if (HAL_TIM_ConfigClockSource(&htim, &sClockConfig)) {
        return -1;
    }

    TIM_MasterConfigTypeDef sMasterConfig
        = {.MasterOutputTrigger = TIM_TRGO_UPDATE,
           .MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE};
    if (HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig)) {
        return -1;
    }

    if (HAL_TIM_Base_Start(&htim)) {
        return -1;
    }

    return 0;
}

static int dac_dma_init() {
    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA2_Stream4_IRQn interrupt configuration */
    // HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 0, 0);
    // HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);

    hdma.Instance = DMA1_Stream5;
    hdma.Init.Channel = DMA_CHANNEL_7;
    hdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma.Init.Mode = DMA_CIRCULAR;
    hdma.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma) != HAL_OK) {
        return -1;
    }

    __HAL_LINKDMA(&hdac, DMA_Handle1, hdma);

    return 0;
}

int audio_start(uint16_t* buffer, uint32_t bufSize) {
    return HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)buffer, bufSize,
                             DAC_ALIGN_12B_R);
}

void audio_stop(void) { HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1); }
