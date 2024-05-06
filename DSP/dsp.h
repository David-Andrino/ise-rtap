#ifndef DSP_H
#define DSP_H

/**
 * @file dsp.h
 *
 * @brief Modulo de procesado digital de se�ales
 *
 * @author Ruben Agustin
 * @author David Andrino
 * @author Estela Mora
 * @author Fernando Sanz
 *
 * Utilizando la librer�a CMSIS DSP, procesa la se�al de entrada,
 * generando la se�al de salida.
 *
 */

#include <stdint.h>

/**
 * Inicializa el modulo de procesado digital.
 *
 * @return int 0 si se ha realizado correctamente. Otro valor si no.
 **/
extern int dsp_init(void);

/**
 * Realiza el procesado de un bloque de datos.
 *
 * @param in  Puntero al buffer de entrada a procesar
 * @param out Puntero al buffer en el que almacenar la salida
 **/
extern void processSamples(uint16_t* in, uint16_t* out);

/**
 * Cambia la configuraci�n del filtrado.
 *
 * @param bands Array de 5 elementos con las ganancias de cada banda.
 *              Elementos enteros en el rango [-9, 9]
 * @param vol   Volumen de salida, en d�cimas. Valor entero en [0, 10]
 **/
extern void dsp_configure_filters(int8_t* bands, uint8_t vol);

#endif
