#ifndef _GUI_H
#define _GUI_H

#include "gui_threads.h"

void lv_gui(void);
//void lv_demo_widgets_start_slideshow(void);

typedef struct{
	uint8_t vol;					  		// Volumen general del sistema
	/* Radio */
	uint32_t freq_khz;      		// La frecuencia de la radio sintonizada
	/* MP3 */
	char songs[200];						// Todas las canciones, separadas por '\n'
	/* Filtros */
	char fcentral[5][10];				// La frecuencia central de cada banda, tal y como será representada (pasar 3.5kHz en vez de 3500Hz)
} gui_data_t;

#endif
