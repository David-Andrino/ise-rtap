#ifndef RADIO_H
#define RADIO_H

/**
 * @file radio.h
 *
 * @brief Modulo del Sintonizador FM
 *
 * @author Ruben Agustin
 * @author David Andrino
 * @author Estela Mora
 * @author Fernando Sanz
 *
 * Sintoniza las emisoras FM deseadas
 */

#include <cmsis_os2.h>
#include <stdint.h>

/**
 * @brief Encender la radio
 */
#define POWERON  0    

/**
 * @brief Apagar la radio
 */
#define POWEROFF 1

/**
 * @brief Hacer un SeekUp
 */
#define SEEKUP   2

/**
 * @brief Hacer un SeekDown
 */
#define SEEKDOWN 3

/**
 * @brief Mandar frecuencia sintonizada
 */
#define INFO     4
// Resto de valores, sintonizar

/**
 * @brief Mensajes que se mandan al sintonizador
 */
typedef uint32_t radioMsg_t;

/**
 * @brief Thread responsable de gestionar el Sintonizador
 */
extern osThreadId_t radio_tid;

/**
 * @brief Cola de mensajes por la que el sintonizador recibe los mensajes
 * del thread principal
 */
extern osMessageQueueId_t mainToRadioQueue;

/**
 * Inicializa el modulo del sintonizador FM. Debe haberse inicializado
 * el protector de I2C previamente.
 *
 * @return 0 si se ha realizado correctamente. Otro valor si no.
 **/
extern int Init_Radio(void);

#endif
