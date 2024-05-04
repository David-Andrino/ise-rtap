#ifndef MP3_H
#define MP3_H

/**
 * @file
 *
 * @brief Modulo del Reproductor MP3
 *
 * @author Ruben Agustin
 * @author David Andrino
 * @author Estela Mora
 * @author Fernando Sanz
 *
 * Lee las canciones de la tarjeta SD situada en el propio reproductor
 */

#include "../main.h"
#include "Driver_USART.h"
#include "cmsis_os2.h"

// ==================================== MP3 MSG TYPES ======================================
/**
 * @brief Enumeración de los tipos de mensaje de entrada al módulo del MP3
 */
typedef enum {
    MP3_NEXT_SONG = 256,
    MP3_PREV_SONG,
    MP3_VOL_UP,
    MP3_VOL_DOWN,
    MP3_SLEEP_MODE,
    MP3_WAKE_UP,
    MP3_RESET,
    MP3_PLAY,
    MP3_PAUSE,
    MP3_STOP_PLAY,
    MP3_START_CYCLE,
    MP3_STOP_CYCLE,
    MP3_ENA_DAC,
    MP3_DIS_DAC
} mp3Msg_t;

/**
 * @brief Thread responsable de gestionar el Reproductor MP3
 */
extern osThreadId_t       MP3_Thread;

/**
 * @brief Cola de mensajes por la que el MP3 recibe los mensajes del thread
 * principal
 */
extern osMessageQueueId_t MP3Queue;

/**
 * Inicializa el modulo del MP3
 *
 * @param none
 *
 * @return 0 si se ha realizado correctamente. Otro valor si no.
 **/
extern int Init_MP3(void);

#endif
