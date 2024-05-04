#ifndef _GUI_H
#define _GUI_H

#include "gui_threads.h"

void lv_gui(void);
//void lv_demo_widgets_start_slideshow(void);
void async_cb(void * data);
void consumo_async_cb(void * data);

typedef struct{
	uint8_t vol;					  		// Volumen general del sistema
	/* Radio */
	uint32_t freq;      		    // La frecuencia de la radio sintonizada, en centenas de kHz
	/* MP3 */
	uint8_t num_canciones;
	char songs[10][30];					// Todas las canciones
	/* Filtros */
	char fcentral[5][10];				// La frecuencia central de cada banda, tal y como será representada (pasar 3.5kHz en vez de 3500Hz)
} gui_data_t;

#endif
