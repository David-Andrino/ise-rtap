#include "cmsis_os2.h"                          
#include "ThreadTest.h"
 
osThreadId_t tid_Test;                       
 
void ThreadTest (void *argument);                   
 
int Init_Test (void) {
 
  tid_Test = osThreadNew(ThreadTest, NULL, NULL);
  if (tid_Test == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThreadTest (void *argument) {
 
	mp3Msg_t msg = {0};
	
	msg = MP3_WAKE_UP;
	osMessageQueuePut(MP3Queue, &msg, NULL, osWaitForever);
	
	msg = 01;
	osMessageQueuePut(MP3Queue, &msg, NULL, osWaitForever);
	
  while (1) {
    osThreadYield();                            
  }
}
