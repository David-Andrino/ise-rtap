#ifndef AUDIO_CONFIG_H
#define AUDIO_CONFIG_H

/**
 * @file audioConfig.h
 *
 * @brief Fichero de configuración del camino analógico de audio
 *
 * @author Ruben Agustin
 * @author David Andrino
 * @author Estela Mora
 * @author Fernando Sanz
 *
 * Permite configurar:
 * - Tamaño de buffer de procesado
 * - GPIO, ADC y DMA de entrada
 * - GPIO, DAC y DMA de salida
 * - TIM de muestreo
 *
 */
#define DSP_BUFSIZE 2048

#define SAMPLING_GPIO_PIN        GPIO_PIN_6
#define SAMPLING_GPIO_PORT       GPIOA
#define SAMPLING_DMA_IRQn        DMA2_Stream4_IRQn
#define SAMPLING_DMA_ISR         DMA2_Stream4_IRQHandler
#define __SAMPLING_GPIO_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define __SAMPLING_ADC_ENABLE()  __HAL_RCC_ADC1_CLK_ENABLE()
#define __SAMPLING_DMA_ENABLE()  __HAL_RCC_DMA2_CLK_ENABLE()
#define SAMPLING_ADC             ADC1
#define SAMPLING_ADC_CHANNEL     ADC_CHANNEL_6
#define SAMPLING_DAC_TRIGGER     ADC_EXTERNALTRIGCONV_T2_TRGO
#define SAMPLING_DMA_INSTANCE    DMA2_Stream4
#define SAMPLING_DMA_CHANNEL     DMA_CHANNEL_0
#define SAMPLING_DMA_HANDLE      DMA_Handle

#define AUDIO_DAC             DAC1
#define AUDIO_DAC_CHANNEL     DAC_CHANNEL_1
#define AUDIO_GPIO_PORT       GPIOA
#define AUDIO_GPIO_PIN        GPIO_PIN_4
#define __AUDIO_GPIO_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define __AUDIO_DAC_ENABLE()  __HAL_RCC_DAC_CLK_ENABLE()
#define __AUDIO_DMA_ENABLE()  __HAL_RCC_DMA1_CLK_ENABLE()
#define AUDIO_DAC_TRIGGER     DAC_TRIGGER_T2_TRGO
#define AUDIO_DMA_INSTANCE    DMA1_Stream5
#define AUDIO_DMA_CHANNEL     DMA_CHANNEL_7
#define AUDIO_DMA_HANDLE      DMA_Handle1

/*  El timer va al APB1 (que va a 1/4 de la freq del procesador)
 *  El procesador va a 216 MHz.
 *  La freq del tim2 es: 216M / ((9 + 1) * (224 + 1)) = 24kHz
 */
#define DSP_TIM_INSTANCE   TIM2 
#define __DSP_TIM_ENABLE() __HAL_RCC_TIM2_CLK_ENABLE()
#define DSP_TIM_PRESCALER  4
#define DSP_TIM_PERIOD     224

#endif
