#include "cmsis_os2.h"                          
#include "ThreadTestWeb.h"
#include "ThreadWeb.h"
#include "../Control/controlThread.h"
 
osThreadId_t tid_TestWeb;                        // thread id
 
void ThreadTest (void *argument);                   // thread function
 
int Init_WebTest (void) {
 
  tid_TestWeb = osThreadNew(ThreadTest, NULL, NULL);
  if (tid_TestWeb == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThreadTest (void *argument) {
    uint8_t cnt = 0;
    web_out_msg_t msg = {0};
    msg_ctrl_t msgOut = {0};
    
    for (int i = 0;; i = (i + 10) % 2000){
        msg.type = WEB_OUT_DATE;
        msg.payload = i; 
        osMessageQueuePut(webQueue, &msg, 0, osWaitForever);
        msg.type = WEB_OUT_CONS;
        msg.payload = i; 
        osMessageQueuePut(webQueue, &msg, 0, osWaitForever);
        osDelay(500);
    }
    
    msg.type = WEB_OUT_INPUT_SEL;
    msg.payload = 1;
    osMessageQueuePut(webQueue, &msg, 0, osWaitForever);
     
    osDelay(2000);
    
    msg.type = WEB_OUT_INPUT_SEL;
    msg.payload = 0;
    osMessageQueuePut(webQueue, &msg, 0, osWaitForever);
     
    osDelay(2000);
    
    msg.type = WEB_OUT_INPUT_SEL;
    msg.payload = 0;
    osMessageQueuePut(webQueue, &msg, 0, osWaitForever);
     
    osDelay(2000);
    
    msg.type = WEB_OUT_OUTPUT_SEL;
    msg.payload = 0;
    osMessageQueuePut(webQueue, &msg, 0, osWaitForever);
     
    osDelay(2000);
    
    msg.type = WEB_OUT_OUTPUT_SEL;
    msg.payload = 1;
    osMessageQueuePut(webQueue, &msg, 0, osWaitForever);
     
    osDelay(2000);
    
    msg.type = WEB_OUT_VOL;
    msg.payload = 5;
    osMessageQueuePut(webQueue, &msg, 0, osWaitForever);
     
    osDelay(2000);
    
    msg.type = WEB_OUT_CONS;
    msg.payload = 1956;
    osMessageQueuePut(webQueue, &msg, 0, osWaitForever);
     
    osDelay(2000);
    
    msg.type = WEB_OUT_RADIO_FREQ;
    msg.payload = 1025;
    osMessageQueuePut(webQueue, &msg, 0, osWaitForever);
     
    osDelay(2000);
    
    msg.type = WEB_OUT_BANDS;
    msg.payload = 0x0207;
    osMessageQueuePut(webQueue, &msg, 0, osWaitForever);
     
    osDelay(2000);    
    
    msg.type = WEB_OUT_BANDS;
    msg.payload = 0x04FE;
    osMessageQueuePut(webQueue, &msg, 0, osWaitForever);
     
    osDelay(2000);
    
    msg.type = WEB_OUT_DATE;
    msg.payload = 0x1D0601;
    osMessageQueuePut(webQueue, &msg, 0, osWaitForever);
     
    osDelay(2000); 
    
    msg.type = WEB_OUT_HOUR;
    msg.payload = 0x0A1E15;
    osMessageQueuePut(webQueue, &msg, 0, osWaitForever);
     
    osDelay(2000);    
      
  while (1) {
    osMessageQueueGet(ctrl_in_queue, &msgOut, 0, osWaitForever);
    osDelay(1000);
  }
}
