#ifndef MP3_H
#define MP3_H

/**
 * @file MP3.h
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
 * @brief Enumeración de los tipos de mensaje de entrada al módulo del MP3. Otro valor
 *        implica un cambio de frecuencia
 */
typedef enum {
    MP3_NEXT_SONG = 256,                /**< Siguiente canción */        
    MP3_PREV_SONG,                      /**< Anterior canción */
    MP3_VOL_UP,                         /**< Subir volumen */
    MP3_VOL_DOWN,                       /**< Bajar volumen */
    MP3_SLEEP_MODE,                     /**< Activar sleep mode */
    MP3_WAKE_UP,                        /**< Desactivar sleep mode */
    MP3_RESET,                          /**< Realizar un reset */
    MP3_PLAY,                           /**< Reproducir la canción */
    MP3_PAUSE,                          /**< Pausar la canción */
    MP3_STOP_PLAY,                      /**< Para de reproducir */
    MP3_START_CYCLE,                    /**< Poner canción en bucle */
    MP3_STOP_CYCLE,                     /**< Descativar canción en bucle */
    MP3_ENA_DAC,                        /**< Habilitar el DAC */
    MP3_DIS_DAC                         /**< Deshabilitar el DAC */
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
 * @return int 0 si se ha realizado correctamente. Otro valor si no.
 **/
extern int Init_MP3(void);

#endif
