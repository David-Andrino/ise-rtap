#ifndef DAC_H
#define DAC_H
#include <stdint.h>

/***************************************************************************//**
 * @file
 * 
 * @brief Modulo de salida de audio de RTAP
 * 
 * @author Ruben Agustin
 * @author David Andrino
 * @author Estela Mora
 * @author Fernando Sanz
 * 
 * Utiliza un DAC disparado por un timer para generar muestras a 48 kHz. 
 * Mediante DMA, obtiene los valores de un buffer.
 * 
 * Perifericos utilizados:
 * - TIM2
 * - GPIO PA4
 * - DAC1
 */

/***************************************************************************//**
 * Inicializa el modulo de salida de audio.
 * 
 * @return 0 si se ha realizado correctamente. Otro valor si no.
 ******************************************************************************/
int  audio_init();

/***************************************************************************//**
 * Arranca la salida de muestras del buffer.
 * @param buffer Zona de memoria donde se encuentran las muestras
 * @param bufSize Muestras maximas del buffer
 * @return 0 si se ha realizado correctamente. Otro valor si no.
 ******************************************************************************/
int  audio_start(uint16_t* buffer, uint32_t bufSize);

/***************************************************************************//**
 * Detiene la generacion de muestras.
 ******************************************************************************/
void audio_stop(void);
    
#endif