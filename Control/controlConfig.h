/**
 * @file
 *
 * @brief Fichero de configuración del módulo de control
 *
 * @author Ruben Agustin
 * @author David Andrino
 * @author Estela Mora
 * @author Fernando Sanz
 *
 * Permite configurar el módulo de control. Particularmente, gestiona el
 * rango de frecuencias de la radio, los pines de habilitación y 
 * selección de entrada y salida y el pin de la medición de consumo.
 *
 */

#ifndef CONTROL_CONFIG_H
#define CONTROL_CONFIG_H

#define RADIO_MAX_FREQ 1080
#define RADIO_MIN_FREQ 870

#define SELECT_INPUT_RADIO GPIO_PIN_SET
#define SELECT_INPUT_MP3   GPIO_PIN_RESET

#define SELECT_OUTPUT_EAR GPIO_PIN_SET
#define SELECT_OUTPUT_SPK GPIO_PIN_RESET

#define ENA_GPIO_ON  GPIO_PIN_SET
#define ENA_GPIO_OFF GPIO_PIN_RESET

#define ENA_GPIO_PIN  GPIO_PIN_3
#define ENA_GPIO_PORT GPIOJ
#define __ENA_GPIO()  __HAL_RCC_GPIOJ_CLK_ENABLE();

#define INPUT_SELECT_GPIO_PIN  GPIO_PIN_7
#define INPUT_SELECT_GPIO_PORT GPIOF
#define __INPUT_SELECT_GPIO()  __HAL_RCC_GPIOF_CLK_ENABLE();

#define OUTPUT_SELECT_GPIO_PIN  GPIO_PIN_8
#define OUTPUT_SELECT_GPIO_PORT GPIOC
#define __OUTPUT_SELECT_GPIO()  __HAL_RCC_GPIOC_CLK_ENABLE();

#define CONS_GPIO_PIN        GPIO_PIN_10
#define CONS_GPIO_PORT       GPIOF
#define __CONS_ENABLE_GPIO() __HAL_RCC_GPIOF_CLK_ENABLE();

#define __ENA_CONS_ADC() __HAL_RCC_ADC3_CLK_ENABLE();
#define CONS_ADC         ADC3
#define CONS_ADC_CHANNEL ADC_CHANNEL_8

#endif
