#ifndef SD_H
#define SD_H

#include <stdint.h>

typedef struct {
	int8_t bands[5];
	uint8_t volume;
} sd_config_t;

/**
  * @brief  Init SD
  * @param  None
	* @retval 0 all correct, 1 error
  */
uint8_t Init_SD(char SD_Songs[][30], int* numSongs, sd_config_t* msg);

/**
  * @brief  Save config to SD
  * @param  None
  * @retval 0 all correct, 1 error
  */
uint8_t Save_Config(sd_config_t* msg);

#endif
