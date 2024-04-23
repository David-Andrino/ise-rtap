#include "cmsis_os2.h"  
#include "ThreadTest.h"
#include "radio.h"
#include <stdio.h>

 
osThreadId_t tid_Test; 

static uint32_t freq_actual;

 
void ThreadTest (void *argument);                   
 
int Init_Test (void) {
 
  tid_Test = osThreadNew(ThreadTest, NULL, NULL);
  if (tid_Test == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThreadTest (void *argument) {
	
	size_t tmp = sizeof(radioMsg_t);
	
	radioResponse_t msgRadioMain = {0};
	radioMsg_t msgMainRadio = 0;
	printf("========== COMIENZO DE LOS TEST ==========\n");
	
	printf("========== ENCENDIDO DE LA RADIO ==========\n");
	msgMainRadio = POWERON;
	osMessageQueuePut(mainToRadioQueue, &msgMainRadio, NULL, osWaitForever);
	osDelay(500);
	
	printf("========== SINTONIZAR FRECUENCIA ==========\n");
	uint32_t freq = 97000;
	osMessageQueuePut(mainToRadioQueue, &freq, NULL, osWaitForever);
	
	msgMainRadio = INFO;
	osMessageQueuePut(mainToRadioQueue, &msgMainRadio, NULL, osWaitForever);
	osDelay(500);
	osMessageQueueGet(radioToMainQueue, &msgRadioMain, NULL, osWaitForever);
	freq_actual = msgRadioMain.freq;
	printf("La frecuencia sintonizada es: %0.1d\n", freq_actual);
	printf("La potencia de la frecuencia sintonizada es: %d\n", msgRadioMain.power);
	if (freq_actual == 97000){
		printf("========== TEST CORRECTO ==========\n");
	} else {
		printf("========== TEST INCORRECTO ==========\n");
	}
	
	printf("========== ESPERAMOS 5 SEGUNDOS ==========\n");
	osDelay(5000);
	
	printf("========== HACEMOS UN SEEKUP ==========\n");
	msgMainRadio = SEEKUP;
	osMessageQueuePut(mainToRadioQueue, &msgMainRadio, NULL, osWaitForever);
	osDelay(500);
	msgMainRadio = INFO;
	osMessageQueuePut(mainToRadioQueue, &msgMainRadio, NULL, osWaitForever);
	osDelay(500);
	osMessageQueueGet(radioToMainQueue, &msgRadioMain, NULL, osWaitForever);
	printf("La frecuencia sintonizada es: %0.1d\n", msgRadioMain.freq);
	printf("La potencia de la frecuencia sintonizada es: %d\n", msgRadioMain.power);
	if (msgRadioMain.freq != freq_actual){
		freq_actual = msgRadioMain.freq; 
		printf("========== TEST CORRECTO ==========\n");
	} else {
		printf("========== TEST INCORRECTO ==========\n");
	}
	
	printf("========== ESPERAMOS 5 SEGUNDOS ==========\n");
	osDelay(5000);
	
	printf("========== HACEMOS UN SEEKDOWN ==========\n");
	msgMainRadio = SEEKDOWN;
	osMessageQueuePut(mainToRadioQueue, &msgMainRadio, NULL, osWaitForever);
	osDelay(500);
	msgMainRadio = INFO;
	osMessageQueuePut(mainToRadioQueue, &msgMainRadio, NULL, osWaitForever);
	osDelay(500);
	osMessageQueueGet(radioToMainQueue, &msgRadioMain, NULL, osWaitForever);
	printf("La frecuencia sintonizada es: %0.1d\n", msgRadioMain.freq);
	printf("La potencia de la frecuencia sintonizada es: %d\n", msgRadioMain.power);
	if (msgRadioMain.freq != freq_actual){
		freq_actual = msgRadioMain.freq; 
		printf("========== TEST CORRECTO ==========\n");
	} else {
		printf("========== TEST INCORRECTO ==========\n");
	}
	
	printf("========== ESPERAMOS 10 SEGUNDOS ==========\n");
	osDelay(10000);
	
	
	printf("========== APAGAMOS LA RADIO ==========\n");	
	msgMainRadio = POWEROFF;
	osMessageQueuePut(mainToRadioQueue, &msgMainRadio, NULL, osWaitForever);
	
	while(1){
		osThreadYield();
	}
}