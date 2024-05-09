#ifndef __SD
#define __SD

#include "stdint.h"

typedef struct {
	int8_t bands[5]; 	/**< Bandas de frecuencia*/
	uint8_t volume; 	/**< Volumen*/
} sd_config_t;

int Init_SD();  																		/**< Inicializa la tarjeta sd */
int read_file  (char *file, char *buf); 	   				/**< Lee un fichero y guarda el contenido en un buffer */
int write_file (char *file, char *buf, int size);		/**< Escribe en un fichero el contenido de un buffer de un tamaño especifico */
int append_file(char *file, char *buf, int size);		/**< Añade a un fichero el contenido de un buffer de un tamaño especifico */
#endif
