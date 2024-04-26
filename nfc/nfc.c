#include "nfc.h"

osThreadId_t nfc_tid; 
osMessageQueueId_t nfcToMainQueue;
extern ARM_DRIVER_I2C Driver_I2C2;
static ARM_DRIVER_I2C* I2Cdrv2 = &Driver_I2C2;
static volatile uint32_t I2C_Event;

GPIO_InitTypeDef GPIO_InitStruct;
 
void ThreadNFC (void *argument);    
static void I2C2_callback(uint32_t event);

const uint8_t selectSystemFile[10] = {0x03, 0x00, 0xA4, 0x00,
																			0x0C, 0x02, 0xE1, 0x01,
																			0xC0, 0x8C};

const uint8_t readSystemFile[8] = {0x03, 0x00, 0xB0, 0x00,
																	0x00, 0x12, 0xC1, 0x69};
															
uint8_t readSystemFileResponse[18];
uint8_t writeSystemFileResponse[18];
uint8_t verifyResponse[18];
																	
const uint8_t writeSystemFile[9] = {0x03, 0x00, 0xD6, 0x00,
																	0x03, 0x01, 0x10, 0xDB, 0x0D};

const uint8_t verify[136] = {0x03, 0x00, 0x20, 0x00,
																	0x03, 0x10, 
																	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																	0x9A, 0x77};

const uint8_t openSession[1] = {0x26};
	
const uint8_t selectNFC[16] = {0x02, 0x00, 0xA4, 0x04,
															 0x00, 0x07, 0xD2, 0x76,
															 0x00, 0x00, 0x85, 0x01,		
															 0x01, 0x00, 0x35, 0xC0};

const uint8_t selectCC[10] = {0x03, 0x00, 0xA4, 0x00,
															0x0C, 0x02, 0xE1, 0x03,
															0xD2, 0xAF};
	
const uint8_t readCCLength[8] = {0x02, 0x00, 0xB0, 0x00,
																 0x00, 0x02, 0x6B, 0x7D};

const uint8_t readCCFile[8] = {0x03, 0x00, 0xB0, 0x00,
													     0x00, 0x0F, 0xA5, 0xA2};

const	uint8_t selectNDEF[10] = {0x02, 0x00, 0xA4, 0x00,
														    0x0C, 0x02, 0x00, 0x01,
														    0x3E, 0xFD};	

const uint8_t readNDEFLength[8] = {0x03, 0x00, 0xB0, 0x00,
																	 0x00, 0x02, 0x40, 0x79};

const uint8_t readNDEF[8] = {0x02, 0x00, 0xB0, 0x00,
												     0x02, 0x14, 0x6C, 0x3B};

const uint8_t deselect[3] = {0xC2, 0xE0, 0xB4};

uint8_t readNDEFResponse[20];

uint8_t readCCFileResponse[20];

int Init_NFC (void) {
 
  nfc_tid = osThreadNew(ThreadNFC, NULL, NULL);
  if (nfc_tid == NULL) {
    return(-1);
  }
	
	nfcToMainQueue = osMessageQueueNew(10, sizeof(nfcReading_t), NULL);
	if (nfcToMainQueue ==NULL){
		return (-1);
	}
 
  return(0);
}
 
void ThreadNFC (void *argument) {
	I2C_Init();
	GPIO_Init();
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET); //Habilitar RF - Deshabilitar I2C
	
	Leer_NFC();
	
  while (1) {
		
		osThreadFlagsWait(NFC_FLAG, osFlagsWaitAll, osWaitForever); //Esperar inicio RF
		
		osDelay(500); //Esperar fin transferencia RF
		
		osThreadFlagsClear(NFC_FLAG); //Limpiar flag
		
		Leer_NFC(); //Leer contenido NDEF NFC
  }
}

int I2C_Init(void){
	int error = 0;	
	error |= I2Cdrv2->Initialize((ARM_I2C_SignalEvent_t)I2C2_callback);
	error |= I2Cdrv2->PowerControl(ARM_POWER_FULL);
	error |= I2Cdrv2->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
	error |= I2Cdrv2->Control(ARM_I2C_BUS_CLEAR, 0);	
	return error;
}

void GPIO_Init(void){
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
	
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void EXTI15_10_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	osThreadFlagsSet(nfc_tid, NFC_FLAG);
}

void Leer_NFC(void){
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET); //Deshabilitar RF - Habilitar I2C
	
	OpenSession();	
	SelectNFC();	
	Verify();
	SelectFileSystem();
  WriteFileSystem();
	SelectFileSystem();
	ReadFileSystem();
	/* SelectCC();
  ReadCCLength();
  ReadCCFile();
  SelectNDEF();
  ReadNDEFLength();
  ReadNDEF(); */
  Deselect();
	
	osThreadFlagsClear(NFC_FLAG); //Limpiar flag antes de habilitar el RF para que vuelva a medir I2C
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET); //Habilitar RF - Deshabilitar I2C
}

void OpenSession(void){
	
  I2Cdrv2->MasterTransmit(0x56, openSession, 1, false); 
	while(I2Cdrv2->GetStatus().busy);													
	osDelay(10);					 

}

void Verify(void){													 
	I2Cdrv2->MasterTransmit(0x56, verify, 136, false);	
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(100);
													 
	I2Cdrv2->MasterReceive(0x56,verifyResponse,5,false);
  while(I2Cdrv2->GetStatus().busy);	
  osDelay(10);												 									
}

void SelectNFC(void){	
	uint8_t selectNFCResponse[5];
													 
	I2Cdrv2->MasterTransmit(0x56, selectNFC, 16, false);	
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);
													 
	I2Cdrv2->MasterReceive(0x56,selectNFCResponse,5,false);
  while(I2Cdrv2->GetStatus().busy);	
  osDelay(10);												 									
}

void SelectFileSystem(void){
	uint8_t selectFileSystemResponse[10];			 
	 
	I2Cdrv2->MasterTransmit(0x56, selectSystemFile, 10, false);	
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);
													 
	I2Cdrv2->MasterReceive(0x56,selectFileSystemResponse,10,false);
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);										
}

void ReadFileSystem(void){
	
	I2Cdrv2->MasterTransmit(0x56, readSystemFile, 8, false);	
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);
													 
	I2Cdrv2->MasterReceive(0x56,readSystemFileResponse,18,false);
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);											
}

void WriteFileSystem(void){
	
	I2Cdrv2->MasterTransmit(0x56, writeSystemFile, 9, false);	
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);
													 
	I2Cdrv2->MasterReceive(0x56,writeSystemFileResponse,18,false);
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);											
}

void SelectCC(void){
	uint8_t selectCCResponse[5];			 
	 
	I2Cdrv2->MasterTransmit(0x56, selectCC, 10, false);	
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);
													 
	I2Cdrv2->MasterReceive(0x56,selectCCResponse,5,false);
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);										
}

void ReadCCLength(void){
	uint8_t readCCLengthResponse[7];
	
	I2Cdrv2->MasterTransmit(0x56, readCCLength, 8, false);	
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);
													 
	I2Cdrv2->MasterReceive(0x56,readCCLengthResponse,7,false);
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);										
}

void ReadCCFile(void){
	
	I2Cdrv2->MasterTransmit(0x56, readCCFile, 8, false);	
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);
													 
	I2Cdrv2->MasterReceive(0x56,readCCFileResponse,20,false);
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);											
}

void SelectNDEF(void){
	uint8_t selectNDEFResponse[5];
	
	I2Cdrv2->MasterTransmit(0x56, selectNDEF, 10, false);	
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);
													 
	I2Cdrv2->MasterReceive(0x56,selectNDEFResponse,5,false);
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);										
}

void ReadNDEFLength(void){
	uint8_t readNDEFLengthResponse[7];
	
	I2Cdrv2->MasterTransmit(0x56, readNDEFLength, 8, false);	
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);
													 
	I2Cdrv2->MasterReceive(0x56,readNDEFLengthResponse,7,false);
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);											
}

void ReadNDEF(void){
	I2Cdrv2->MasterTransmit(0x56, readNDEF, 8, false);	
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);
													 
	I2Cdrv2->MasterReceive(0x56,readNDEFResponse,20,false);
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);											
}

void Deselect(void){
	uint8_t deselectResponse[3];
	
	I2Cdrv2->MasterTransmit(0x56, deselect, 3, false);	
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);
													 
	I2Cdrv2->MasterReceive(0x56,deselectResponse,3,false);
  while(I2Cdrv2->GetStatus().busy);	
	osDelay(10);
}

void I2C2_callback(uint32_t event){
	/* Save received events */
  I2C_Event |= event;
 
  /* Optionally, user can define specific actions for an event */
  osThreadFlagsSet(nfcToMainQueue, 0x20);
  if (event == ARM_I2C_EVENT_TRANSFER_INCOMPLETE) {
    /* Less data was transferred than requested */
		osThreadFlagsSet(nfcToMainQueue, 0x20);
  }
 
  if (event == ARM_I2C_EVENT_TRANSFER_DONE) {
    /* Transfer or receive is finished */
		osThreadFlagsSet(nfcToMainQueue, 0x20);
  }
 
  if (event == ARM_I2C_EVENT_ADDRESS_NACK) {
    /* Slave address was not acknowledged */
		osThreadFlagsSet(nfcToMainQueue, 0x20);
  }
 
  if (event & ARM_I2C_EVENT_ARBITRATION_LOST) {
    /* Master lost bus arbitration */
  }
 
  if (event & ARM_I2C_EVENT_BUS_ERROR) {
    /* Invalid start/stop position detected */
  }
 
  if (event & ARM_I2C_EVENT_BUS_CLEAR) {
    /* Bus clear operation completed */
  }
 
  if (event & ARM_I2C_EVENT_GENERAL_CALL) {
    /* Slave was addressed with a general call address */
  }
 
  if (event & ARM_I2C_EVENT_SLAVE_RECEIVE) {
    /* Slave addressed as receiver but SlaveReceive operation is not started */
  }
 
  if (event & ARM_I2C_EVENT_SLAVE_TRANSMIT) {
    /* Slave addressed as transmitter but SlaveTransmit operation is not started */
  }
}
