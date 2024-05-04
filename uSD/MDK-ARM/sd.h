#ifndef SD_H
#define SD_H

/**
 * @file
 *
 * @brief Modulo SD
 *
 * @author Ruben Agustin
 * @author David Andrino
 * @author Estela Mora
 * @author Fernando Sanz
 *
 * Utilizando la librer�a FatFs, se inicializa la SD y se leen 
 * los archivos .txt que contienen la lista de canciones y la 
 * configuraci�n. Adem�s, se guarda en el archivo .txt correspondiente
 * la configuraci�n actual cuando se le indica.
 *
 */

#include <stdint.h>


/**
 * @brief Estructura para la informaci�n de configuraci�n
 */
typedef struct {
	int8_t bands[5]; /**< Bandas de frecuencia*/
	uint8_t volume; /**< Volumen*/
} sd_config_t;


/**
 * Inicializa la tarjeta SD.
 *
 * @return 0 si se ha realizado correctamente. Otro valor si no.
 **/
uint8_t Init_SD(char SD_Songs[][30], int* numSongs, sd_config_t* msg);


/**
 * Guarda la configuraci�n actual en el archivo .txt correspondiente.
 *
 * @param in  Puntero al buffer que contiene la informaci�n a guardar
 * @return 0 si se ha realizado correctamente. Otro valor si no.
 **/
uint8_t Save_Config(sd_config_t* msg);

#endif
