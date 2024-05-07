#include "adc.h"

#include "../DSP/audioConfig.h"
#include "../main.h"

static ADC_HandleTypeDef hadc = {0};
static DMA_HandleTypeDef hdma = {0};

static int convs = 0, value = 0;

static int adc_init(void);
static int adc_dma_init(void);

static volatile sampling_Callback halfCb = NULL;
static volatile sampling_Callback fullCb = NULL;

void DMA2_Stream4_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma); }

int sampling_init(sampling_Callback firstHalfCb,
                  sampling_Callback secondHalfCb) {
    if (firstHalfCb != NULL)
        halfCb = firstHalfCb;
    if (secondHalfCb != NULL)
        fullCb = secondHalfCb;

    return adc_init() | adc_dma_init();
}

static int adc_init(void) {
    __SAMPLING_GPIO_ENABLE();
    GPIO_InitTypeDef gpioADC = {
        .Pin = SAMPLING_GPIO_PIN,
        .Mode = GPIO_MODE_ANALOG,
        .Pull = GPIO_NOPULL,
    };
    HAL_GPIO_Init(SAMPLING_GPIO_PORT, &gpioADC);

    __SAMPLING_ADC_ENABLE();
    hadc.Instance = SAMPLING_ADC;
    hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.ScanConvMode = DISABLE;
    hadc.Init.ContinuousConvMode = DISABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConv = SAMPLING_DAC_TRIGGER;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.NbrOfConversion = 1;
    hadc.Init.DMAContinuousRequests = ENABLE;
    hadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    if (HAL_ADC_Init(&hadc)) {
        return -1;
    }

    ADC_ChannelConfTypeDef sConfig = {
        .Channel = SAMPLING_ADC_CHANNEL,
        .Rank = 1,
        .SamplingTime = ADC_SAMPLETIME_56CYCLES,
    };
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig)) {
        return -1;
    }

    return 0;
}

static int adc_dma_init() {
    /* DMA controller clock enable */
    __SAMPLING_DMA_ENABLE();

    /* DMA interrupt init */
    /* DMA2_Stream4_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SAMPLING_DMA_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SAMPLING_DMA_IRQn);

    hdma.Instance = SAMPLING_DMA_INSTANCE;
    hdma.Init.Channel = SAMPLING_DMA_CHANNEL;
    hdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
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

    __HAL_LINKDMA(&hadc, SAMPLING_DMA_HANDLE, hdma);

    return 0;
}

int sampling_start(uint16_t* buffer, uint32_t bufSize) {
    return HAL_ADC_Start_DMA(&hadc, (uint32_t*)buffer, bufSize);
}

void ADC_IRQHandler(void) { HAL_ADC_IRQHandler(&hadc); }

void sampling_stop(void) { HAL_ADC_Stop_DMA(&hadc); }

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
    if (halfCb != NULL)
        halfCb();
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (fullCb != NULL)
        fullCb();
}
