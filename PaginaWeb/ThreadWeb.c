#include "cmsis_os2.h"                          
#include "ThreadWeb.h"
 
#define MAX_MSG 8 
 
osThreadId_t tid_Web;    
osMessageQueueId_t mainToWebQueue;
 
void Thread_Web (void *argument);                   
 
int Init_Web (void) {
 
  tid_Web = osThreadNew(Thread_Web, NULL, NULL);
  if (tid_Web == NULL) {
    return(-1);
  }
	
	mainToWebQueue = osMessageQueueNew(MAX_MSG, sizeof(webout_msg_t), NULL);
	if (mainToWebQueue == NULL){
		return -1;
	}
 
  return(0);
}
 
void Thread_Web (void *argument) {
 
	webout_msg_t msgMainToWeb = {0};
	
  while (1) {
		osMessageQueueGet(mainToWebQueue, &msgMainToWeb, NULL, osWaitForever);
		
  }
}
