#include "sd.h"

#include <stdio.h>                      // Standard I/O .h-file
#include <ctype.h>                      // Character functions
#include <string.h>                     // String and memory functions

#include "../main.h"

#include "rl_fs.h"                      // Keil.MDK-Pro::File System:CORE

// Main stack size must be multiple of 8 Bytes. No lo uso
#define APP_SD_STK_SZ (1200U)
uint64_t app_sd_stk[APP_SD_STK_SZ / 8];
const osThreadAttr_t app_sd_attr = {
  .stack_mem  = &app_sd_stk[0],
  .stack_size = sizeof(app_sd_stk)
};

enum {BACKSPACE = 0x08,
      LF        = 0x0A,
      CR        = 0x0D,
      CNTLQ     = 0x11,
      CNTLS     = 0x13,
      ESC       = 0x1B,
      DEL       = 0x7F };

/* Funciones privadas */
static char *get_entry         (char *cp, char **pNext);
static int   write_append_file (char *file, char *buf, bool append, int size);
uint8_t      Read_Songs();
uint8_t      Write_Conf();
// __NO_RETURN void thread_sd (void *arg);


/**
  *	@brief  Inizializa la sd
	* @param  Void
	* @retval 0 si ok, -1 si error
	*/
int Init_SD(){
	fsStatus stat;
	int ret = 0;
  /* Initialize and mount drive "M0" */
  stat = finit ("M0:");
  if (stat == fsOK) {
    stat = fmount ("M0:");
    if (stat == fsOK) {
			ret = 0; // Todo oku
    } else if (stat == fsNoFileSystem) { // Se ha ido el sistema de ficheros, ponerlo con el ordenador y no comerse la cabeza
			ret = -1;
    } else { // Tamos mal
      ret = -1;
    }
  }
  else {
		// Disaster happened
		return -1;
  }
	return ret;
}



int read_file (char *file, char *buf) {
  char *fname,*next;
  FILE *f;
  int ch;

  fname = get_entry (file, &next);
  if (fname == NULL) {
    printf ("\nFilename missing.\n");
    return 0;
  }
  
  f = fopen (fname,"r");                /* open the file for reading          */
  if (f == NULL) {
    printf ("\nFile not found!\n");
    return 0;
  }
	
	int i = 0;
  while ((ch = fgetc (f)) != EOF) {     /* read the characters from the file  */
		buf[i++] = ch;
	}
  fclose (f);                           /* close the input file when done     */
  printf ("\nFile closed.\n");
	
	return i; // Numero de caracteres que ha leído
}

/**
  *	@brief  Escribe un fichero
	* @param  Fichero que se quiere escribir
  * @param  Buffer con la informacion
	* @param  Tama�o del buffer
	* @retval 0 si ok, -1 si error
	*/
int write_file (char *file, char *buf, int size){
	return write_append_file(file, buf, false, size);
}

/**
  *	@brief  A�ade a un fichero
	* @param  Fichero que se quiere escribir
  * @param  Buffer con la informacion
	* @param  Tama�o del buffer
	* @retval 0 si ok, -1 si error
	*/
int append_file(char *file, char *buf, int size){
	return write_append_file(file, buf, true, size);
}

static int write_append_file (char *file, char *buf, bool append, int size) {
  char *fname, *next;
  bool  retv;
  FILE *f;

  fname = get_entry (file, &next);
  if (fname == NULL) {
    printf ("\nFilename missing.\n");
    return -1;
  }
  f = fopen (fname,append ? "a" : "w"); /* open a file for writing            */
  if (f == NULL) {
    return -1;
  }
	
	int i = 0;
	while((buf[i] != NULL) && (i < size)){  // No esta detectando el fin de un string
		fputc(buf[i++], f);
	}
  fclose (f);                         /* close the output file                */
  return ferror(f);
}
/*-----------------------------------------------------------------------------
 *        Process input string for long or short name entry
 *----------------------------------------------------------------------------*/
static char *get_entry (char *cp, char **pNext) {
  char *sp, lfn = 0, sep_ch = ' ';

  if (cp == NULL) {                           /* skip NULL pointers           */
    *pNext = cp;
    return (cp);
  }

  for ( ; *cp == ' ' || *cp == '\"'; cp++) {  /* skip blanks and starting  "  */
    if (*cp == '\"') { sep_ch = '\"'; lfn = 1; }
    *cp = 0;
  }
 
  for (sp = cp; *sp != CR && *sp != LF && *sp != 0; sp++) {
    if ( lfn && *sp == '\"') break;
    if (!lfn && *sp == ' ' ) break;
  }

  for ( ; *sp == sep_ch || *sp == CR || *sp == LF; sp++) {
    *sp = 0;
    if ( lfn && *sp == sep_ch) { sp ++; break; }
  }

  *pNext = (*sp) ? sp : NULL;                 /* next entry                   */
  return (cp);
}


//void Init_Thread_SD(){
//	osThreadId_t sd = osThreadNew(thread_sd, NULL, NULL);
//}
//__NO_RETURN void thread_sd (void *arg) {
//	char canciones[500] = "", contenido_test_1[] = "test\n";
//	int tam_test = sizeof(contenido_test_1)/sizeof(char) - 1;
//  
//	Init_SD ();
//  read_file("RTAP/Songs.txt", canciones);
//	write_file("TEST.o", contenido_test_1, tam_test);
//	append_file("TEST.o", canciones, 500);
//  while (1) {
//    osThreadYield();
//  }
//}

int SD_read_songs(char *file, char songs[][30]) {
    char SD_Data[25 * 30] = { 0 };
    int bytesRead = 0;
    if ((bytesRead = read_file("RTAP/songs.txt", SD_Data)) <= 0) {
        return -1;
    }
    
    uint8_t  numSongs   = 0;
	int i, j = 0;
    
    for(i = 0; i < bytesRead; i++){
		if(SD_Data[i] == '\n'){
			while(j<30){
				songs[numSongs][j] = '\0';
				j++;
			}
			if(numSongs == 24){
				break;
			}
			numSongs++;
			j = 0;
		} else if(SD_Data[i] != '\r') {
		  songs[numSongs][j] = SD_Data[i];
			j++;
		}	
        
	}
    
    return numSongs+1;
}


int SD_read_config(sd_config_t* config) {
    char tmpBuf[18];
	
	if(read_file("RTAP/Config.txt", tmpBuf) == 0){
		config->bands[0] = 0;
		config->bands[1] = 0;
		config->bands[2] = 0;
		config->bands[3] = 0;
		config->bands[4] = 0;
	
		config->volume = 10;
        return -1;
	}
	
	sscanf(tmpBuf, "%2hhd %2hhd %2hhd %2hhd %2hhd %2hhu", &config->bands[0], &config->bands[1], &config->bands[2], &config->bands[3], &config->bands[4], &config->volume);
	
	for(int i = 0; i < 5; i++){
		if(config->bands[i] < -9){
			config->bands[i] = -9;
		}
	
		if(config->bands[i] > 9){
			config->bands[i] = 9;
		}
	}
		
	if(config->volume < 0){
		config->volume = 0;
	}
	
	if(config->volume > 10){
		config->volume = 10;
	}

	return 0;
}

int SD_write_config(sd_config_t* config) {
	char tmpBuf[18];
	
	snprintf((char*)tmpBuf, 18, "%2hhd %2hhd %2hhd %2hhd %2hhd %2hhu", config->bands[0], config->bands[1], config->bands[2], config->bands[3], config->bands[4], config->volume);
	
	return write_file("RTAP/Config.txt", tmpBuf, 18);
}
