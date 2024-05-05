/**
 * @file controlThread.h
 *
 * @brief Módulo de control de RTAP
 *
 * @author Ruben Agustin
 * @author David Andrino
 * @author Estela Mora
 * @author Fernando Sanz
 *
 * Módulo principal de inteligencia del sistema. Recibe eventos por 
 * una cola y reacciona acorde a ellos
 *
 */
#ifndef CONTROL_THREAD_H
#define CONTROL_THREAD_H

#include <cmsis_os2.h>
#include <stdint.h>

#include "../main.h"

/**
 * @brief Cola de mensajes de entrada al módulo
 */
extern osMessageQueueId_t ctrl_in_queue;

/**
 * @brief Inicialización del módulo de control
 *
 * @return 0 si se ha realizado correctamente. Otro valor si no.
 */
int Init_Control(void);

// ==================================== MSG TYPES ======================================
/**
 * @brief Enumeración de los tipos de mensaje de entrada al módulo de control
 */
typedef enum {
    MSG_NFC,   /**< Lectura de una tarjeta del NFC */
    MSG_LCD,   /**< Mensaje de entrada del LCD     */
    MSG_WEB,   /**< Mensaje de entrada de la web   */
    MSG_RTC,   /**< Mensaje de entrada del RTC     */
    MSG_CONS,  /**< Mensaje de entrada del consumo */
    MSG_RADIO, /**< Mensaje de entrada de la radio */
} msg_ctrl_type_t;

// ===================================== LCD ======================================
/**
 * @brief Enumeración de mensajes de entrada del LCD
 */
typedef enum {
    LCD_VOL,        /**< Cambio de volumen. Contenido es el volumen */
    LCD_BANDS,      /**< Cambio de filtro. Contenido es primer byte la banda [0,4] segundo la cantidad [-9, 9] */
    LCD_RADIO_FREQ, /**< Cambio de frecuencia de la radio. Contenido es la frecuencia en centenas de kHz */
    LCD_SONG,       /**< Cambio de canción. Contenido es el número de canción */
    LCD_INPUT_SEL,  /**< Cambio de entrada. Contenido es 0 para la radio y 1 para MP3 */
    LCD_OUTPUT_SEL, /**< Cambio de salida. Contenido es 0 para cascos y 1 para altavoz */
    LCD_SAVE_SD,    /**< Guardar configuración en la SD. Contenido ignorado */
    LCD_LOW_POWER,  /**< Entrar en modo bajo consumo. Contenido ignorado */
    LCD_LOOP,       /**< Poner canción en bucle. Contenido ignorado*/
    LCD_SEEK,       /**< Hacer seek con la radio. Contenido es 0 para down y 1 para up */
    LCD_NEXT_SONG,  /**< Siguiente canción */
    LCD_PREV_SONG,  /**< Anterior canción */
    LCD_PLAY_PAUSE, /**< Alternar reproducción de la canción */
} lcd_msg_type_t;

/**
 * @brief Estructura para los mensajes de entrada del LCD
 */
typedef struct {
    lcd_msg_type_t type;    /**< Tipo de mensaje del LCD */
    uint16_t       payload; /**< Contenido del mensaje. Depende del tipo. */
} lcd_msg_t;

// ==================================== WEB =======================================
/**
 * @brief Enumeración de mensajes de entrada de la web
 */
typedef enum {
    WEB_INPUT_SEL,  /**< Cambio de entrada. Contenido es 0 para la radio y 1 para MP3 */
    WEB_OUTPUT_SEL, /**< Cambio de salida. Contenido es 0 para cascos y 1 para altavoz */
    WEB_LOW_POWER,  /**< Entrar en modo bajo consumo. Contenido ignorado */
    WEB_RADIO_FREQ, /**< Cambio de frecuencia de la radio. Contenido es la frecuencia en centenas de kHz */
    WEB_SEEK,       /**< Hacer seek con la radio. Contenido es 0 para down y 1 para up */
    WEB_VOL,        /**< Cambio de volumen. Contenido es el volumen */
    WEB_SONG,       /**< Cambio de canción. Contenido es el número de canción */
    WEB_PLAY_PAUSE, /**< Alternar play y pause de la web */
    WEB_PREV_SONG,  /**< Anterior canción */
    WEB_NEXT_SONG,  /**< Siguiente canción*/
    WEB_BANDS,      /**< Cambio de filtro. Contenido es primer byte la banda [0,4] segundo la cantidad [-9, 9] */
    WEB_SAVE_SD,    /**< Guardar configuración en la SD. Contenido ignorado */
    WEB_LOOP,       /**< Poner canción en bucle. Contenido ignorado*/
} web_msg_type_t;

/**
 * @brief Estructura para los mensajes de entrada del LCD
 */
typedef struct {
    web_msg_type_t type;    /**< Tipo del mensaje de */
    uint16_t       payload; /**< Contenido del mensaje. Depende del tipo */
} web_msg_t;

// ====================================== NFC ========================================
/**
 * @brief Estructura para los mensajes de entrada del NFC
 */
typedef struct {
    uint8_t  type;    /**< Tipo de mensaje. 0 para canción y 1 para radio*/
    uint16_t content; /**< Numero de canción o frecuencia en centenas*/
} nfc_msg_t;

// ====================================== RTC ========================================
/**
 * @brief Estructura para los mensajes de entrada del RTC
 */
typedef struct {
    uint8_t hour, minute, second, day, month, year;
} rtc_msg_t;

// ====================================== MSG ========================================
/**
 * @brief Estructura para los mensajes de entrada
 */
typedef struct {
    msg_ctrl_type_t type;    /**< Tipo de mensaje de entrada. 
                                  Dependiendo de este valor se debe interpretar el contenido */
    union {
        nfc_msg_t nfc_msg;   /**< Contenido de un mensaje de tipo MSG_NFC   */
        lcd_msg_t lcd_msg;   /**< Contenido de un mensaje de tipo MSG_LCD   */
        rtc_msg_t rtc_msg;   /**< Contenido de un mensaje de tipo MSG_RTC   */
        web_msg_t web_msg;   /**< Contenido de un mensaje de tipo MSG_WEB   */
        uint16_t  cons_msg;  /**< Contenido de un mensaje de tipo MSG_CONS  */
        uint32_t  radio_msg; /**< Contenido de un mensaje de tipo MSG_RADIO */
    };
} msg_ctrl_t;

#endif
