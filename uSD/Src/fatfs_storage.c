#include "fatfs_storage.h"

uint8_t sector[512];
FATFS fs;
FILINFO fno;
DIR dir;
FIL F;

uint32_t BytesRead = 0;
uint32_t BytesWrite = 0;

uint32_t Storage_Init(void)
{
  BSP_SD_Init();
  
  /****************** FatFs Volume Access ******************************/
  if(f_mount(&fs, (TCHAR const*)"",0)){
    return 1;
  }
	
  return 0;
}


uint32_t Storage_OpenReadFile(uint8_t *Address, const char* TxtName)
{
 
	FIL F1;

  if (f_open(&F1, (TCHAR const*)TxtName, FA_READ) != FR_OK){
    return 0;
  }
	
  if (f_read (&F1, Address, 750, (UINT *)&BytesRead) != FR_OK){
    return 0;
  }
  
  f_close (&F1);
  
  return BytesRead;
}


uint32_t Storage_OpenWriteFile(uint8_t *Address, const char* TxtName)
{
  FIL F1;

  if (f_open(&F1, (TCHAR const*)TxtName, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK){
    return 0;
  }
	
	if (f_lseek(&F1, 0) != FR_OK){
    return 0;
  }
	
  if (f_write (&F1, Address, strlen((char*)Address), (UINT *)&BytesWrite) != FR_OK){
    return 0;
  }

  f_close (&F1);
  
  return BytesWrite;
}


uint32_t Storage_CheckBitmapFile(const char* TxtName, uint32_t *FileLen)
{
  if(f_mount(&fs, (TCHAR const*)"",0)){
    return 1;
  }
	
  if(f_open (&F, (TCHAR const*)TxtName, FA_READ)){
    return 2;
  }
  
  f_read (&F, sector, 6, (UINT *)&BytesRead);

  return 0;
}


uint32_t Storage_GetDirectoryBitmapFiles (const char* DirName, char* Files[])
{
  FRESULT res;
  uint32_t index = 0;

  /* Open filesystem */
  if(f_mount(&fs, (TCHAR const*)"",0) != FR_OK){
    return 0;
  }

  /* Start to search for wave files */
  res = f_findfirst(&dir, &fno, DirName, "*.txt");

  /* Repeat while an item is found */
  while (fno.fname[0]){
    if(res == FR_OK){
      if(index < MAX_TXT_FILES){
        sprintf (Files[index++], "%s", fno.fname);
      }
      /* Search for next item */
      res = f_findnext(&dir, &fno);
    }
    else{
      index = 0;
      break;
    }
  }

  f_closedir(&dir);

  return index;
}
