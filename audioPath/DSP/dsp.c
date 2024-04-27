#include "../main.h"
#include "dsp.h"
#include "arm_math.h"

static TIM_HandleTypeDef htim = {0};

int dsp_tim_init(void) {
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

void processSamples(uint16_t* in, uint16_t* out) {
    static uint16_t yprev = 0;
    
    for (int i = 0; i < DSP_BUFSIZE / 2; i++) {
        yprev = (in[i] + yprev)/2;
        out[i] = yprev;
    }
}