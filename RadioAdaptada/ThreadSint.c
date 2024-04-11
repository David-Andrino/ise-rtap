#include "cmsis_os2.h"                         
#include "ThreadSint.h"

osThreadId_t tid_Sintonizador;  
osMessageQueueId_t msgQueue;
static osTimerId_t timer;

static uint16_t RDA5807M_WriteRegDef[6] ={0xC004,0x0000,0x0100,0x84D4,0x4000,0x0000}; // initial data

static void InitRegis(void);

static volatile uint32_t I2C_Event;

static msg_t msg = {0};

static uint8_t registros_lectura[12];
	
int I2C_Init(void);
void WriteAll(void);
void PowerOn(void);
void Frequency(uint32_t freq_kHz);
void SetVolume(uint8_t vol);
void SeekUp(void);
static void I2C2_callback(uint32_t event);
static void Readregisters(void);
static void timer_callback(void);

uint32_t seeFrec(void);

 
void Thread (void *argument);                   
int Init_Sintonizador (void) {
 
  tid_Sintonizador= osThreadNew(Thread, NULL, NULL);
  if (tid_Sintonizador == NULL) {
    return(-1);
  }
	
	msgQueue = osMessageQueueNew(MAX_MSG, sizeof(msg_t), NULL);
	if (msgQueue == NULL){
		return -1;
	}
	
	timer = osTimerNew((osTimerFunc_t) &timer_callback, osTimerPeriodic, NULL, NULL);
	if (timer == NULL)
		return -1;

  return(0);
}
 
void Thread (void *argument) {
	
	I2C_Init();
	InitRegis();
	
	osMessageQueueGet(msgQueue, &msg, NULL, osWaitForever);
 
	PowerOn();	
	Readregisters();
	WriteAll();
	
	RDA5807M_WriteReg[0] = RDA5807M_WriteReg[0] | RDA_RDS_ON;
	
	
	osTimerStart(timer, 500U);
	
	Frequency(msg.freq);
	Readregisters();
	
	osMessageQueueGet(msgQueue, &msg, NULL, osWaitForever);
	
	Frequency(msg.freq);
	Readregisters();
	
  while (1) {
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

void InitRegis(void){
	for (int i =0;i < 6;i++){
		RDA5807M_WriteReg[i] = RDA5807M_WriteRegDef[i];
	}
	WriteAll();
}

void SeekUp(void){
	RDA5807M_WriteReg[0] = RDA5807M_WriteReg[0] | RDA_SEEK_UP;
	WriteAll();
	RDA5807M_WriteReg[0] = RDA5807M_WriteReg[0] & RDA_SEEK_STOP;
}

void WriteAll(void){
    uint8_t buffer[12];  
    int i,x = 0;
    for(i=0; i<12; i=i+2){
        buffer[i] = RDA5807M_WriteReg[x] >> 8;
        x++;
		}
    x = 0;
    for(i=1; i<13; i=i+2){
        buffer[i] = RDA5807M_WriteReg[x] & 0xFF;
        x++;
		}
    I2Cdrv2->MasterTransmit(0x10, buffer, 12, false); 
		while(I2Cdrv2->GetStatus().busy);
}

void PowerOn(void){
    RDA5807M_WriteReg[1] = RDA5807M_WriteReg[1] | RDA_TUNE_ON;
    RDA5807M_WriteReg[0] = RDA5807M_WriteReg[0] | RDA_POWER;
    WriteAll(); 
    RDA5807M_WriteReg[1] = RDA5807M_WriteReg[1] & 0xFFEF;   //Disable tune after PowerOn operation
}

void Frequency(uint32_t freq_kHz){
//	float frecuencia = freq_kHz / 1000;									 // Pasa a Hz 
	uint16_t Channel;
  Channel = ((((float)freq_kHz/1000)-StartingFreq)/0.1)+0.05;
	Channel = Channel & 0x3FF;
	RDA5807M_WriteReg[1] = Channel*64 + 0x10;  // Channel + TUNE-Bit + Band=00(87-108) + Space=00(100kHz)
	WriteAll();
	RDA5807M_WriteReg[1] = RDA5807M_WriteReg[1] & RDA_TUNE_OFF;
}

void SetVolume(uint8_t vol){
	
	if(vol > 15)
		vol = 15;
	
	for(int j = 0; j < 6; j++)
		RDA5807M_WriteReg[j] = RDA5807M_WriteRegDef[j];
	
	if(vol != 0) PowerOn();
	else RDA5807M_WriteReg[0] = (RDA5807M_WriteReg[0] & 0xBFFF);		// Mute
	
  PowerOn();
	RDA5807M_WriteReg[3] = (RDA5807M_WriteReg[3] & 0xFFF0)| vol;   // Set New Volume
	WriteAll();
	RDA5807M_WriteReg[0] = RDA5807M_WriteReg[0] | RDA_RDS_ON;
}

void I2C2_callback(uint32_t event){
	/* Save received events */
  I2C_Event |= event;
 
  /* Optionally, user can define specific actions for an event */
  osThreadFlagsSet(tid_Sintonizador, RDA_TRANSFER_COMPLETE);
  if (event == ARM_I2C_EVENT_TRANSFER_INCOMPLETE) {
    /* Less data was transferred than requested */
		osThreadFlagsSet(tid_Sintonizador, RDA_TRANSFER_COMPLETE);
  }
 
  if (event == ARM_I2C_EVENT_TRANSFER_DONE) {
    /* Transfer or receive is finished */
		osThreadFlagsSet(tid_Sintonizador, RDA_TRANSFER_COMPLETE);
  }
 
  if (event == ARM_I2C_EVENT_ADDRESS_NACK) {
    /* Slave address was not acknowledged */
		osThreadFlagsSet(tid_Sintonizador, RDA_TRANSFER_COMPLETE);
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
	
void timer_callback(void){
	Readregisters();
}


void Readregisters(void){
   uint8_t rcv[12];

	 osDelay(50);
	 I2Cdrv2->MasterReceive(0x10,rcv,12,false);
	 while(I2Cdrv2->GetStatus().busy){}
	 for (int i = 0; i < 6;i++){
		 RDA5807M_ReadReg[i] = ((rcv[i*2] << 8) | rcv [(i*2)+1]);
	 }
	 
	 for(int i = 0; i< 12; i++){
		registros_lectura[i] = rcv[i];
	 }
}


uint32_t seeFrec(void){
	Readregisters();
	return (((registros_lectura[0] & 0x03) << 8 | (registros_lectura[1] & 0xFF)) * 100) + 87000;
}


