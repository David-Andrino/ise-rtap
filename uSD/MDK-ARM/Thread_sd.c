#include "Thread_sd.h"                 
#include "sd.h"

osThreadId_t tid_Thread_SD;   
void Thread_SD (void *argument);  

char songs[25][30] = { 0 };
int songCnt = 0;
sd_config_t sd_msg;

void Error_Handler(void);
 
int Init_Thread_SD (void) {
 
  tid_Thread_SD = osThreadNew(Thread_SD, NULL, NULL);
  if (tid_Thread_SD == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_SD (void *argument) {
	
	Init_SD(songs, &songCnt, &sd_msg);
	
	//Config aleatoria
	sd_msg.bands[0] = -1;
	sd_msg.bands[1] = -1;
	sd_msg.bands[2] = -1;
	sd_msg.bands[3] = -1;
	sd_msg.bands[4] = 1;
	
	sd_msg.volume = 7;
	
	if(Save_Config(&sd_msg) == 1){
		Error_Handler();
	}
		
  while (1) {}
}

void Error_Handler(void){
	while(1){}
}
