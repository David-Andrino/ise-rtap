#include "../main.h"
#include "adc.h"
#include <Board_LED.h>

#define ADC_GPIO GPIOA
#define ADC_PIN  GPIO_PIN_3

#define TMP_TIM_GPIO GPIOD
#define TMP_TIM_PIN  GPIO_PIN_12

static ADC_HandleTypeDef hadc    = { 0 };
static TIM_HandleTypeDef htim    = { 0 };
static TIM_HandleTypeDef htmptim = { 0 };
static DMA_HandleTypeDef hdma    = { 0 };

static int convs = 0, value = 0;

static int adc_init(void);
static int adc_tim_init(void);
static int tmp_tim_init(void);
static int adc_dma_init(void);

static void adc_dma_halfCallback(DMA_HandleTypeDef* hdma);
static void adc_dma_fullCallback(DMA_HandleTypeDef* hdma);

void DMA2_Stream4_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma);
}    

int sampling_init() {
    return tmp_tim_init() | 
           adc_tim_init() | 
           adc_init()     | 
           adc_dma_init() ;
}

static int adc_init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef gpioPA3 = {
        .Pin = GPIO_PIN_3,
        .Mode = GPIO_MODE_ANALOG,
        .Pull = GPIO_NOPULL
    };
    HAL_GPIO_Init(GPIOA, &gpioPA3);
    
    __HAL_RCC_ADC1_CLK_ENABLE();
    hadc.Instance = ADC1;
    hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.ScanConvMode = DISABLE;
    hadc.Init.ContinuousConvMode = DISABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.NbrOfConversion = 1;
    hadc.Init.DMAContinuousRequests = ENABLE;
    hadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    if (HAL_ADC_Init(&hadc)) {
        return -1;
    }
    
    ADC_ChannelConfTypeDef sConfig = { 
        .Channel      = ADC_CHANNEL_3,
        .Rank         = 1,
        .SamplingTime = ADC_SAMPLETIME_56CYCLES
    };
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig)) {
        return -1;
    }
    
    return 0;
}

static int tmp_tim_init(void) {
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_TIM4_CLK_ENABLE();
    GPIO_InitTypeDef tmpTimGPIO = {
        .Pin = TMP_TIM_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Alternate = GPIO_AF2_TIM4,
        .Pull = GPIO_NOPULL
    };
    HAL_GPIO_Init(TMP_TIM_GPIO, &tmpTimGPIO);
    
    htmptim.Instance = TIM4;
    htmptim.Init.Prescaler = 999;
    htmptim.Init.Period = 20;
    if (HAL_TIM_OC_Init(&htmptim)) {
        return -1;
    }
    
    TIM_OC_InitTypeDef tmpTimChannel = {
        .OCMode = TIM_OCMODE_TOGGLE,
        .OCPolarity = TIM_OCPOLARITY_HIGH,
        .OCFastMode = TIM_OCFAST_DISABLE
    };
    
    if (HAL_TIM_OC_ConfigChannel(&htmptim, &tmpTimChannel, TIM_CHANNEL_1)) {
        return -1;
    }        

    if (HAL_TIM_OC_Start(&htmptim, TIM_CHANNEL_1)) {
        return -1;
    }
    
    LED_Initialize();
    
    return 0;
}

int adc_tim_init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();
    htim.Instance = TIM2;
    
    htim.Init.Prescaler = 4;
    htim.Init.Period = 174;
    
    if (HAL_TIM_Base_Init(&htim)) {
        return -1;
    }
    
    TIM_ClockConfigTypeDef sClockConfig = { .ClockSource = TIM_CLOCKSOURCE_INTERNAL };
    if (HAL_TIM_ConfigClockSource(&htim, &sClockConfig)) {
        return -1;
    }
    
    TIM_MasterConfigTypeDef sMasterConfig = {
        .MasterOutputTrigger = TIM_TRGO_UPDATE,
        .MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE
    };
    if (HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig)) {
        return -1;
    }
    
    if (HAL_TIM_Base_Start(&htim)) {
        return -1;
    }
    
    return 0;
}

static int adc_dma_init() {
    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA2_Stream4_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);
    
    hdma.Instance                 = DMA2_Stream4;
    hdma.Init.Channel             = DMA_CHANNEL_0;
    hdma.Init.Direction           = DMA_PERIPH_TO_MEMORY;
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

    __HAL_LINKDMA(&hadc,DMA_Handle,hdma);
   
    return 0;
}

int sampling_start(uint16_t* buffer, uint32_t bufSize) {
    return HAL_ADC_Start_DMA(&hadc, (uint32_t*) buffer, bufSize);
}

void ADC_IRQHandler(void) {
    HAL_ADC_IRQHandler(&hadc);
}

void sampling_stop(void) {
    HAL_ADC_Stop_DMA(&hadc);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    guarreria = 2;
    sampling_stop();
}

