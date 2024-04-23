#include "test-adc.h"
#include "adc.h"
#include "../main.h"

#include <string.h>
#include <stdio.h>

static volatile int done = 0;
static uint16_t  buffer[2048];
static uint16_t* half_buf = &buffer[1024];

static void samplingFullCb(void);
static int tmp_tim_init(void);
static TIM_HandleTypeDef htmptim = { 0 };

void test_adc() {
  memset(buffer, 0xFA, 2048);
  tmp_tim_init();
  int ret = sampling_init(NULL, samplingFullCb);
  sampling_start(buffer, 2048);
  
  while (!done);

  char line[156] = {0};
  for (int i = 0; i < 50; i++) {
      for (int j = 0; j < 32; j++) {
        snprintf(line + 5*j, 6, "%04.4X ", buffer[32*i + j]);
      }
      printf("%s\n", line);
      fflush(NULL);
      HAL_Delay(100);
  }
}

void samplingFullCb() {
    done = 1;
    sampling_stop();
}


static int tmp_tim_init(void) {
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_TIM4_CLK_ENABLE();
    GPIO_InitTypeDef tmpTimGPIO = {
        .Pin = GPIO_PIN_12,
        .Mode = GPIO_MODE_AF_PP,
        .Alternate = GPIO_AF2_TIM4,
        .Pull = GPIO_NOPULL
    };
    HAL_GPIO_Init(GPIOD, &tmpTimGPIO);
    
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
    
    return 0;
}