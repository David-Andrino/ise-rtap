#ifndef THREADWEB_H
#define THREADWEB_H

/**
 * @file ThreadWeb.h
 *
 * @brief Modulo del la pagina web
 *
 * @author Ruben Agustin
 * @author David Andrino
 * @author Estela Mora
 * @author Fernando Sanz
 *
 * Pagina web para gestionar la interaccion con la placa
 */

#include "../main.h"
#include "cmsis_os2.h"

#define SONG_NAME_LENGTH 30

/**
 * @brief Lista de canciones
 */
extern char* web_lista_canciones;

/**
 * @brief Numero de canciones
 */
extern int web_cnt_canciones;

/**
 * Inicializa el modulo del la pagina web
 *
 * @param none
 *
 * @return int 0 si se ha realizado correctamente. Otro valor si no.
 **/
extern int Init_Web (char lista_canciones[][SONG_NAME_LENGTH], int cnt_canciones);

/**
 * @brief Cola de mensajes por la que la pagina web recibe los mensajes
 * del thread principal
 */
extern osMessageQueueId_t webQueue;

/**
 * @brief Cerrojo para la estructura de datos que maneja la pagina web
 */
extern osMutexId_t web_mutex;

/**
 * @brief Stack de la pagina web
 */
extern uint64_t app_main_stk[];

/**
 * @brief Atributos de la pagina web
 */
extern const osThreadAttr_t app_main_attr;


/**
 * @brief Funcion principal de la pagina web
 * 
 * @param arg 
 */
extern void app_main (void *arg);

/**
 * @brief Enumeración de las entradas de audio del sistema
 */
typedef enum {
	WEB_RADIO			= 0,        /**< Seleccionar radio como entrada */
	WEB_MP3			 	= 1         /**< Seleccionar MP3 como entrada */
} input_t;

/**
 * @brief Enumeración de las salidas de audio del sistema
 */
typedef enum {
	WEB_AURICULARES 	= 0,        /**< Seleccionar auriculares como salida */
	WEB_ALTAVOZ 		= 1         /**< Seleccionar altavoz como salida */
} output_t;

/**
 * @brief Estructura para los datos que maneja la pagina web
 */
typedef struct {
	input_t entrada;                        /**< Entrada seleccionada */
	output_t salida;                        /**< Salida seleccionada */
	uint8_t bajo_consumo;                   /**< Modo bajo consumo */
	uint8_t vol;                            /**< Volumen seleccionado */
    uint8_t prev_vol;                       /**< Volumen anterior */
	uint8_t mute;                           /**< Estado de mute */
	uint16_t consumo;                       /**< Consumo del sistema */
	uint32_t freq_actual;                   /**< Frecuencia seleccionada */
	int8_t eq1, eq2, eq3, eq4, eq5;         /**< Valores de las bandas */
	uint8_t horas, min, seg;                /**< Hora del sistema */
	uint8_t dia, mes, ano;                  /**< Fecha del sistema */
} web_state_t;

/**
 * @brief Enumeración de mensajes de salida a la web
 */
typedef enum {
	WEB_OUT_INPUT_SEL,      /**< Cambio de entrada. Contenido es 0 para la radio y 1 para MP3 */
	WEB_OUT_OUTPUT_SEL,     /**< Cambio de salida. Contenido es 0 para cascos y 1 para altavoz */
    WEB_OUT_VOL,            /**< Cambio de volumen. Contenido es el volumen [0, 10]*/
	WEB_OUT_CONS,           /**< Medida de consumo. Contenido es mA en [0, 2000] */
    WEB_OUT_RADIO_FREQ,     /**< Cambio de frecuencia de la radio. Contenido es la frecuencia en centenas de kHz */
    WEB_OUT_BANDS,          /**< Cambio de filtro. Contenido es primer byte la banda [0,4] segundo la cantidad [-9, 9] */
    WEB_OUT_DATE,	        /**< Fecha  del sistema*/
    WEB_OUT_HOUR	        /**< Hora  del sistema*/
} web_out_msg_type_t;

/**
 * @brief Estructura para los mensajes de salida a la web
 */
typedef struct {
    web_out_msg_type_t type;        /**< Tipo de mensaje */
    uint32_t payload;               /**< Contenido del mensaje */
}web_out_msg_t;

extern web_state_t web_state;
#endif
