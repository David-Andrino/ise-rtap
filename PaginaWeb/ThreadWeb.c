#include "cmsis_os2.h"                          
#include "ThreadWeb.h"
 
#define MAX_MSG 16
 
osThreadId_t tid_Web;  
osMutexId_t web_mutex;
osMessageQueueId_t webQueue;
 
void Thread_Web (void *argument);                   
 
int Init_Web (void) {
 
  tid_Web = osThreadNew(Thread_Web, NULL, NULL);
  if (tid_Web == NULL) {
    return(-1);
  }
	
	webQueue = osMessageQueueNew(MAX_MSG, sizeof(web_out_msg_t), NULL);
	if (webQueue == NULL){
		return -1;
	}
    
    web_mutex = osMutexNew(NULL);
    if (web_mutex == NULL){
        return -1;
    }
 
  return(0);
}
 
void Thread_Web (void *argument) {
 
	web_out_msg_t msgMainToWeb = {0};
	
  while (1) {
		osMessageQueueGet(webQueue, &msgMainToWeb, NULL, osWaitForever);
        osMutexAcquire(web_mutex, osWaitForever);
		switch (msgMainToWeb.type){
			case WEB_OUT_INPUT_SEL:
                web_state.entrada = msgMainToWeb.payload;
			break;	
			case WEB_OUT_OUTPUT_SEL:
				web_state.salida = msgMainToWeb.payload;
			break;
			case WEB_OUT_VOL:
				web_state.vol = msgMainToWeb.payload; 
			break;
			case WEB_OUT_CONS:
				web_state.consumo = msgMainToWeb.payload;
			break;
			case WEB_OUT_RADIO_FREQ:
				web_state.freq_actual = msgMainToWeb.payload;
			break;
			case WEB_OUT_BANDS:
				switch(msgMainToWeb.payload >> 8){
                    case 0:
                        web_state.eq1 = msgMainToWeb.payload & 0xFF;
                    break;
                    case 1:
                        web_state.eq2 = msgMainToWeb.payload & 0xFF;
                    break;
                    case 2:
                        web_state.eq3 = msgMainToWeb.payload & 0xFF;
                    break;
                    case 3:
                        web_state.eq4 = msgMainToWeb.payload & 0xFF;
                    break;
                    case 4:
                        web_state.eq5 = msgMainToWeb.payload & 0xFF;
                    break;
                    default:
                    break;
                }
			break;
			case WEB_OUT_DATE:
				web_state.dia = msgMainToWeb.payload >> 16;
                web_state.mes = ( msgMainToWeb.payload >> 8 ) & 0xFF;
                web_state.ano = msgMainToWeb.payload & 0xFF;
			break;
            case WEB_OUT_HOUR:
				web_state.seg = msgMainToWeb.payload >> 16;
                web_state.min = ( msgMainToWeb.payload >> 8 ) & 0xFF;
                web_state.horas = msgMainToWeb.payload & 0xFF;				
			break;
            default:
            break;
		}
        osMutexRelease(web_mutex);
  }
}
