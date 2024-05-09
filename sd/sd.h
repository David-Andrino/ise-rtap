#ifndef __SD
#define __SD

#include "stdint.h"

typedef struct {
	int8_t bands[5]; 	/**< Bandas de frecuencia*/
	uint8_t volume; 	/**< Volumen*/
} sd_config_t;

int Init_SD();  																		/**< Inicializa la tarjeta sd */

int SD_read_songs(char *file, char songs[][30]);
int SD_read_config(sd_config_t* config);
int SD_write_config(sd_config_t* config);

#endif
