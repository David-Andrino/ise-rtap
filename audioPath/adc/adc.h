#ifndef ADC_H
#define ADC_H
#include <stdint.h>

/**
 * @file
 *
 * @brief Modulo de adquisicion de datos de RTAP
 *
 * @author Ruben Agustin
 * @author David Andrino
 * @author Estela Mora
 * @author Fernando Sanz
 *
 * Utiliza un ADC disparado por un timer para tomar medidas a 48 kHz. Mediante
 * DMA, copia dichas medidas a un buffer.
 *
 * Perifericos utilizados:
 *   - GPIO PA3
 *   - ADC1 CH3
 *   - TIM2
 *   - DMA2 Stream4
 *
 */

/**
 * @brief Puntero a funcion para los callbacks
 */
typedef void (*sampling_Callback)(void);

/**
 * Inicializa el modulo de adquisicion.
 *
 * @param firstHalfCb Puntero a la funcion a llamar cuando se termine de
 *                    procesar la primera mitad del buffer
 * @param secondHalfCb Puntero a la funcion a llamar cuando se termine de
 *                     procesar la segunda mitad del buffer
 * @return 0 si se ha realizado correctamente. Otro valor si no.
 **/
int sampling_init(sampling_Callback firstHalfCb, sampling_Callback secondHalfCb);

/**
 * Arranca la adquisicion de datos sobre el buffer.
 * @param buffer Zona de memoria donde almacenar las medidas
 * @param bufSize Numero de elementos que caben en la zona
 * @return 0 si se ha realizado correctamente. Otro valor si no.
 **/
int sampling_start(uint16_t* buffer, uint32_t bufSize);

/**
 * Detiene la adquisicion de datos.
 **/
void sampling_stop(void);

#endif
