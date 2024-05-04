#include "sd.h"
#include "fatfs_storage.h"
#include <stdlib.h>

FATFS SD_FatFs;  /* File system object for SD card logical drive */
char SD_Path[4]; /* SD card logical drive path */
char* pDirectoryFiles[MAX_TXT_FILES];
uint8_t  ubNumberOfFiles = 0;
uint32_t uwTxtlen = 0;
uint32_t counter = 0;
char SD_Data[750];
uint8_t str[30];
uint8_t* uwInternalBuffer;

/**
  * @brief  Get songs from SD
  * @param  None
  * @retval Number of songs read, >25 error
  */
static uint8_t Get_Songs(char SD_Songs[][30]);

/**
  * @brief  Get config from SD
  * @param  None
  * @retval 0 all correct, 1 error
  */
static uint8_t Get_Config(sd_config_t* msg);

uint8_t Init_SD(char SD_Songs[][30], int* numSongs, sd_config_t* msg)
{
	uwInternalBuffer = (uint8_t *)SD_Data;
  
  if(FATFS_LinkDriver(&SD_Driver, SD_Path) == 0){
    /* Initialize the Directory Files pointers (heap) ###################*/
    for (counter = 0; counter < MAX_TXT_FILES; counter++){
      pDirectoryFiles[counter] = malloc(MAX_TXT_FILE_NAME);
      if(pDirectoryFiles[counter] == NULL){
        return 1;
      }
    }

    /* Get the TXT file names on root directory */
    ubNumberOfFiles = Storage_GetDirectoryBitmapFiles("/RTAP", pDirectoryFiles);

    if (ubNumberOfFiles == 0){
      for (counter = 0; counter < MAX_TXT_FILES; counter++){
        free(pDirectoryFiles[counter]);
      }
			
      return 1;
    }
  }
  else {
    /* FatFs Initialization Error */
    return 1;
  }
	
	if(Get_Config(msg) == 1){
		return 1;
	}
	
	*numSongs = Get_Songs(SD_Songs);
	if(*numSongs >= 26){
		return 1;
	}
	
	return 0;
}

static uint8_t Get_Config(sd_config_t* msg)
{
	uint32_t bytesRead = 0;
	int i = 0;
	
	sprintf ((char*)str, "RTAP/%-11.11s", pDirectoryFiles[0]);
	
	bytesRead = Storage_OpenReadFile(uwInternalBuffer, (const char*)str);
	if(bytesRead == 0){
		msg->bands[0] = 0;
		msg->bands[1] = 0;
		msg->bands[2] = 0;
		msg->bands[3] = 0;
		msg->bands[4] = 0;
	
		msg->volume = 10;
	}
	
	sscanf(&SD_Data[0], "%2hhd %2hhd %2hhd %2hhd %2hhd %2hhu", 
		&msg->bands[0], &msg->bands[1], &msg->bands[2], &msg->bands[3], &msg->bands[4], &msg->volume);
	
	for(i = 0; i < 5; i++){
		if(msg->bands[i] < -9){
			msg->bands[i] = -9;
		}
	
		if(msg->bands[i] > 9){
			msg->bands[i] = 9;
		}
	}
		
	if(msg->volume < 0){
		msg->volume = 0;
	}
	
	if(msg->volume > 10){
		msg->volume = 10;
	}

	return 0;
}

static uint8_t Get_Songs(char SD_Songs[][30])
{
	uint8_t numSongs = 0;
	uint32_t bytesRead = 0;
	int i, j = 0;
	
	sprintf ((char*)str, "RTAP/%-11.11s", pDirectoryFiles[1]);
	
	bytesRead = Storage_OpenReadFile(uwInternalBuffer, (const char*)str);
	if(bytesRead == 0){
		return 50;
	}
	
	for(i = 0; i < bytesRead; i++){
		if(SD_Data[i] == '\n'){
			while(j<30){
				SD_Songs[numSongs][j] = '\0';
				j++;
			}
			if(numSongs == 24){
				break;
			}
			numSongs++;
			j = 0;
		} else if(SD_Data[i] != '\r') {
		  SD_Songs[numSongs][j] = SD_Data[i];
			j++;
		}	
	}

	return numSongs+1;
}

uint8_t Save_Config(sd_config_t* msg)
{
	uint32_t bytesWrite = 0;
	char write[18];
	
	uwInternalBuffer = (uint8_t*) write;
	
	snprintf((char*)write, 18, "%2hhd %2hhd %2hhd %2hhd %2hhd %2hhu", 
		msg->bands[0], msg->bands[1], msg->bands[2], msg->bands[3], msg->bands[4], msg->volume);
	
	sprintf ((char*)str, "RTAP/%-11.11s", pDirectoryFiles[0]);
	
	bytesWrite = Storage_OpenWriteFile(uwInternalBuffer, (const char*)str);
	if(bytesWrite == 0){
		return 1;
	}
	
	return 0;
}
