#include "cmsis_os2.h"  
#include "ThreadTest.h"
#include "nfc.h"
#include <stdio.h>
 
osThreadId_t tid_Test; 
 
void ThreadTest (void *argument);   

nfcReading_t prueba;
extern osMessageQueueId_t nfcToMainQueue;
 
int Init_Test (void) {
 
  tid_Test = osThreadNew(ThreadTest, NULL, NULL);
  if (tid_Test == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThreadTest (void *argument) {
	
	while(1){
		osMessageQueueGet(nfcToMainQueue, &prueba, 0U, 0U);
		osDelay(1000);
	}
}
