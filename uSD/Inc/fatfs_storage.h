#ifndef __FAT_FS_STORAGE_H
#define __FAT_FS_STORAGE_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include "ff.h"
/* FatFs includes component */
#include "ff_gen_drv.h"
#include "sd_diskio.h"

/**
  * @brief  SDCARD Initialization for FatFs
  * @param  None
  * @retval err : Error status (0=> success, 1=> fail)
  */
uint32_t Storage_Init (void);

/**
  * @brief  Abre un archivo txt, lee el contenido y lo guarda en un buffer
  * @param  Address: Buffer donde se guarda todo el contenido leido
  * @param  TxtName: Nombre del archivo .txt a abrir
  * @retval err: Error status (0=> success, 1=> fail)
  */
uint32_t Storage_OpenReadFile(uint8_t *Address, const char* TxtName);

/**
  * @brief  Abre un archivo txt y escribe el contenido de un buffer en él
  * @param  Address: Buffer donde está guardada la información a escribir
  * @param  TxtName: Nombre del archivo .txt a abrir
  * @retval err: Error status (0=> success, 1=> fail)
  */
uint32_t Storage_OpenWriteFile(uint8_t *Address, const char* TxtName);

/**
  * @brief  Open a file and copy its content to a buffer
  * @param  DirName: the Directory name to open
  * @param  FileName: the file name to open
  * @param  BufferAddress: A pointer to a buffer to copy the file to
  * @param  FileLen: the File lenght
  * @retval err: Error status (0=> success, 1=> fail)
  */
uint32_t Storage_GetDirectoryBitmapFiles (const char* DirName, char* Files[]);

/**
  * @brief  List up to 25 file on the root directory with extension .TXT
  * @param  None
  * @retval The number of the found files
  */
uint32_t Storage_CheckBitmapFile(const char* TxtName, uint32_t *FileLen);

#define MAX_TXT_FILES     25
#define MAX_TXT_FILE_NAME 11

#ifdef __cplusplus
}
#endif

#endif /* __FAT_FS_STORAGE_H */
