#ifndef THREADWEB_H
#define THREADWEB_H

/**
 * @file
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

/**
 * Inicializa el modulo del la pagina web
 *
 * @param none
 *
 * @return 0 si se ha realizado correctamente. Otro valor si no.
 **/
extern int Init_Web (void);

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
 * @brief Tamaño del stack de la pagina web
 */
extern uint64_t app_main_stk[];

/**
 * @brief Atributo de la pagina web
 */
extern const osThreadAttr_t app_main_attr;


/**
 * Inicializa thread de la pagina web
 **/
extern void app_main (void *arg);

/**
 * @brief Enumeración de las entradas de audio del sistema
 */
typedef enum {
	WEB_RADIO			= 0,
	WEB_MP3			 	= 1
} input_t;

/**
 * @brief Enumeración de las salidas de audio del sistema
 */
typedef enum {
	WEB_ALTAVOZ 		= 0,
	WEB_AURICULARES 	= 1
} output_t;

/**
 * @brief Estructura para los datos que maneja la pagina web
 */
typedef struct {
	input_t entrada;
	output_t salida;
	uint8_t bajo_consumo;
	uint8_t vol, prev_vol;
	uint8_t mute;
	uint16_t consumo;
	uint32_t freq_actual;
	int8_t eq1, eq2, eq3, eq4, eq5;
	uint8_t horas, min, seg;
	uint8_t dia, mes, ano;
} web_state_t;

/**
 * @brief Enumeración de mensajes de salida a la web
 */
typedef enum {
	WEB_OUT_INPUT_SEL,      /* Cambio de entrada. Contenido es 0 para la radio y 1 para MP3 */
	WEB_OUT_OUTPUT_SEL,     /* Cambio de salida. Contenido es 0 para cascos y 1 para altavoz */
    WEB_OUT_VOL,            /* Cambio de volumen. Contenido es el volumen [0, 10]*/
	WEB_OUT_CONS,           /* Medida de consumo. Contenido es mA en [0, 2000] */
    WEB_OUT_RADIO_FREQ,     /* Cambio de frecuencia de la radio. Contenido es la frecuencia en centenas de kHz */
    WEB_OUT_BANDS,          /* Cambio de filtro. Contenido es primer byte la banda [0,4] segundo la cantidad [-9, 9] */
    WEB_OUT_DATE,	        /* Fecha  del sistema*/
    WEB_OUT_HOUR	        /* Hora  del sistema*/
} web_out_msg_type_t;

/**
 * @brief Estructura para los mensajes de salida a la web
 */
typedef struct {
    web_out_msg_type_t type;
    uint32_t payload;
}web_out_msg_t;

extern web_state_t web_state;
#endif
