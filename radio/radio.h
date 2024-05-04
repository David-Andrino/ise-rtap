#ifndef RADIO_H
#define RADIO_H

/**
 * @file
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

#define POWERON  0
#define POWEROFF 1
#define SEEKUP   2
#define SEEKDOWN 3
#define INFO     4
// Resto de valores, sintonizar

/**
 * @brief Mensajes que se mandan al sintonizador
 */
typedef uint32_t radioMsg_t;

/**
 * @brief Thread responsable de gestionar el Sintonizador
 */
extern osThreadId_t       radio_tid;

/**
 * @brief Cola de mensajes por la que el sintonizador recibe los mensajes
 * del thread principal
 */
extern osMessageQueueId_t mainToRadioQueue;

/**
 * Inicializa el modulo del sintonizador FM
 *
 * @param none
 *
 * @return 0 si se ha realizado correctamente. Otro valor si no.
 **/
extern int Init_Radio(void);

#endif
