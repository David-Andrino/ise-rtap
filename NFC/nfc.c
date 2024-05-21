#include "nfc.h"
#include "Driver_I2C.h"
#include "stm32f7xx_hal.h"
#include <stdio.h>
#include "../Control/controlThread.h"
#include "../i2c/i2c.h"

#define NFC_FLAG 0x01 // Eliminarlo al integrar

osThreadId_t nfc_tid; 
msg_ctrl_t msg;

GPIO_InitTypeDef GPIO_InitStruct;
 
void ThreadNFC (void *argument);    

void GPIO_Init(void);
void Leer_NFC(void);
void OpenSession(void);
void SelectNFC(void);
void SelectNDEF(void);
void ReadNDEFLength(void);
void ReadNDEF(void);
void Deselect(void);
void Enviar_Lectura(void);

const uint8_t openSession[1] = {0x26};
	
const uint8_t selectNFC[16] = {0x02, 0x00, 0xA4, 0x04,
															 0x00, 0x07, 0xD2, 0x76,
															 0x00, 0x00, 0x85, 0x01,		
															 0x01, 0x00, 0x35, 0xC0};

const	uint8_t selectNDEF[10] = {0x02, 0x00, 0xA4, 0x00,
														    0x0C, 0x02, 0x00, 0x01,
														    0x3E, 0xFD};	

const uint8_t readNDEFLength[8] = {0x03, 0x00, 0xB0, 0x00,
																	 0x00, 0x02, 0x40, 0x79};

const uint8_t readNDEF[8] = {0x02, 0x00, 0xB0, 0x00,
												     0x02, 0x0D, 0x2C, 0xB6};

const uint8_t deselect[3] = {0xC2, 0xE0, 0xB4};

char readNDEFResponse[20];

int Init_NFC (void) {
 
  nfc_tid = osThreadNew(ThreadNFC, NULL, NULL);
  if (nfc_tid == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThreadNFC (void *argument) {
	GPIO_Init();
	
	HAL_GPIO_WritePin(DIS_GPIO_PORT, DIS_GPIO_PIN, DIS_GPIO_OFF); //Habilitar RF - Deshabilitar I2C
	
  while (1) {
		
		osThreadFlagsWait(NFC_FLAG, osFlagsWaitAll, osWaitForever); //Esperar inicio RF
		
		osDelay(500); //Esperar fin transferencia RF
		
		osThreadFlagsClear(NFC_FLAG); //Limpiar flag
		
		Leer_NFC(); //Leer contenido NDEF NFC
		
		Enviar_Lectura(); //Formatear la lectura y enviarla por la cola
		
		osThreadFlagsClear(NFC_FLAG); //Limpiar flag antes de habilitar el RF para que vuelva a medir I2C
  }
}

void GPIO_Init(void){
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	
	GPIO_InitStruct.Pin = GPO_GPIO_PIN; 
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	
	HAL_GPIO_Init(GPO_GPIO_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = DIS_GPIO_PIN; 
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
	
	HAL_GPIO_Init(DIS_GPIO_PORT, &GPIO_InitStruct);
}

void EXTI15_10_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(GPO_GPIO_PIN);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == GPO_GPIO_PIN){
		osThreadFlagsSet(nfc_tid, NFC_FLAG);
	}
}

void Leer_NFC(void){
  HAL_GPIO_WritePin(DIS_GPIO_PORT, DIS_GPIO_PIN, DIS_GPIO_ON); //Deshabilitar RF - Habilitar I2C
	
	OpenSession();	 // Empezar trasferencia I2C
	SelectNFC();	 // Protocolo NFC
  SelectNDEF();      // Memoria del periferico (donde están los datos)
  ReadNDEFLength();  // Con nuestros mensajes: 13 bytes siempre. Cosas del NFC (8 bytes) + Letra (R de radio o S de song), espacio e índice
  ReadNDEF();        // Aqui lees los 13 bytes de antes
  Deselect();        // Acabar trasferencia I2C

	HAL_GPIO_WritePin(DIS_GPIO_PORT, DIS_GPIO_PIN, DIS_GPIO_OFF); //Habilitar RF - Deshabilitar I2C
}

void OpenSession(void){
	
  i2c_MasterTransmit(0x56, openSession, 1, false); 												
}

void SelectNFC(void){	
	uint8_t selectNFCResponse[5];
													 
	i2c_MasterTransmit(0x56, selectNFC, 16, false);	
	osDelay(5);   // Tiempo para que el periferico procese los datos
	i2c_MasterReceive(0x56,selectNFCResponse,5,false);

}

void SelectNDEF(void){
	uint8_t selectNDEFResponse[5];
	
	i2c_MasterTransmit(0x56, selectNDEF, 10, false);		
	osDelay(5);   // Tiempo para que el periferico procese los datos
	i2c_MasterReceive(0x56,selectNDEFResponse,5,false);						
	
}

void ReadNDEFLength(void){
	uint8_t readNDEFLengthResponse[7];
	
	i2c_MasterTransmit(0x56, readNDEFLength, 8, false);			
	osDelay(5);   // Tiempo para que el periferico procese los datos
	i2c_MasterReceive(0x56,readNDEFLengthResponse,7,false);	

}

void ReadNDEF(void){
	uint8_t* bufferLectura = (uint8_t*) readNDEFResponse;
	
	i2c_MasterTransmit(0x56, readNDEF, 8, false);	
	osDelay(5);   // Tiempo para que el periferico procese los datos
	i2c_MasterReceive(0x56,bufferLectura,18,false);		

}

void Deselect(void){
	uint8_t deselectResponse[3];
	
	i2c_MasterTransmit(0x56, deselect, 3, false);	
	osDelay(5);   // Tiempo para que el periferico procese los datos
	i2c_MasterReceive(0x56,deselectResponse,3,false);

}

void Enviar_Lectura(void){
	msg.type = MSG_NFC;
	
	if(readNDEFResponse[8] == 'S'){
		msg.nfc_msg.type = 0; //Song
	} else {
		msg.nfc_msg.type = 1; //Radio
	}
	
	sscanf(&readNDEFResponse[10], "%4hd", &msg.nfc_msg.content);
	
	osMessageQueuePut(ctrl_in_queue, &msg, 0U, 0U);
}
