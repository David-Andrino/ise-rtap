#include "cmsis_os2.h"                          
#include "ThreadTest.h"
#include "Board_Buttons.h"
#include <stdio.h>
 
osThreadId_t tid_Test;                       
 
void ThreadTest (void *argument); 
static void waitConfirmation(void);
 
int Init_Test (void) {
 
  tid_Test = osThreadNew(ThreadTest, NULL, NULL);
  if (tid_Test == NULL) {
    return(-1);
  }   
	
	if (Buttons_Initialize() == -1){
		return (-1);
	}
	
 
  return(0);
}
 
void ThreadTest (void *argument) {
 
	mp3Msg_t msg = {0};
	
	printf("========== COMIENZO DEL TEST ==========\n");
	
	msg = MP3_WAKE_UP;
	osMessageQueuePut(MP3Queue, &msg, NULL, osWaitForever);
	
	printf("========== PONEMOS LA PRIMERA CANCION ==========\n");
	
	msg = 05;
	osMessageQueuePut(MP3Queue, &msg, NULL, osWaitForever);
	waitConfirmation();
	
	printf("========== PAUSA ==========\n");
	msg = MP3_PAUSE;
	osMessageQueuePut(MP3Queue, &msg, NULL, osWaitForever);
	waitConfirmation();
	
	printf("========== VOLVEMOS A PONER LA CANCION  ==========\n");
	msg = MP3_PLAY;
	osMessageQueuePut(MP3Queue, &msg, NULL, osWaitForever);
	waitConfirmation();
	
	printf("========== PONEMOS SIGUIENTE CANCION ==========\n");
	msg = MP3_NEXT_SONG;
	osMessageQueuePut(MP3Queue, &msg, NULL, osWaitForever);
	waitConfirmation();
	
	printf("========== VOLVEMOS A LA ANTERIOR CANCION ==========\n");
	msg = MP3_PREV_SONG;
	osMessageQueuePut(MP3Queue, &msg, NULL, osWaitForever);
	waitConfirmation();
	
	printf("========== DEJAMOS EN BUCLE ESTA CANCION ==========\n");
	msg = MP3_START_CYCLE;
	osMessageQueuePut(MP3Queue, &msg, NULL, osWaitForever);
	waitConfirmation();
	
	printf("========== FIN DEL TEST ==========\n");
	msg = MP3_SLEEP_MODE;
	osMessageQueuePut(MP3Queue, &msg, NULL, osWaitForever);	
	
  while (1) {
    osThreadYield();                            
  }
}

void waitConfirmation(void){
	printf("Para pasar a la siguiente prueba, pulsar el boton B1\n");	
	do {		
	} while(Buttons_GetState() == 0);
	osDelay(500);
}
