#ifndef THREADWEB_H
#define THREADWEB_H

#include "main.h"

extern int Init_Web (void);

typedef enum {
	RADIO			= 0,
	MP3			 	= 1
} input_t;

typedef enum {
	ALTAVOZ 			= 0,
	AURICULARES 	= 1
} output_t;

typedef struct {
	input_t entrada;
	output_t salida;
	uint8_t bajo_consumo;
	uint8_t vol, prev_vol;
	uint8_t mute;
	float consumo;
	uint32_t freq_actual;
	int8_t eq1, eq2, eq3, eq4, eq5;
	uint8_t horas, min, seg;
	uint8_t dia, mes, ano;
} options;


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
		WEB_OUT_TIME_DATE				/* Fecha y hora sel sistema*/
} web_out_msg_type_t;

/**
 * @brief Estructura para los mensajes de salida a la web
 */
typedef struct {
    web_out_msg_type_t type;
    uint16_t payload;
}webout_msg_t;
#endif
