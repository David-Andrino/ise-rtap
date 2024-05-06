/**
 * @file dspThread.h
 *
 * @brief Módulo de procesado digital de señales
 *
 * @author Ruben Agustin
 * @author David Andrino
 * @author Estela Mora
 * @author Fernando Sanz
 *
 * Módulo encargado de la gestión de la configuración de 
 * los filtros digitales y el volumen de salida.
 * 
 */
#ifndef DSP_THREAD_H
#define DSP_THREAD_H

#include <cmsis_os2.h>

/**
 * @brief Número de mensajes en la cola de mensajes
 */
#define DSP_MSG_CNT 16

/**
 * @brief Estructura de configuración de filtros 
 */
typedef struct {
    uint8_t vol;          /**< Volumen de salida. Entre 0 y 10, incluidos */
    int8_t  bandGains[5]; /**< Ganancia en dB de cada banda. Entre -9 y 9 incluidos */
} dspMsg_t;

/**
 * @brief Cola de mensajes para los cambios de configuración
 */
extern osMessageQueueId_t dspQueue;

/**
 * @brief Inicializar el hilo encargado de la configuración de los filtros
 * 
 * @return int 0 si se ha realizado correctamente. Otro valor si no.
 */
extern int DSP_Init(void);

#endif
