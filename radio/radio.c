#include "radio.h"

#include "Driver_I2C.h"
#include "cmsis_os2.h"

#define RDA_TUNE_ON   0x0010
#define RDA_TUNE_OFF  0xFFEF
#define RDA_POWER     0x0001
#define RDA_RDS_ON    0x0008
#define RDA_SEEK_UP   0x0300
#define RDA_SEEK_DOWN 0x0100
#define RDA_SEEK_STOP 0xFCFF

#define StartingFreq  87.00
#define InitialVolume 15

#define MAX_MSG 10

#define RDA_TRANSFER_COMPLETE 0x20

osThreadId_t             radio_tid;
osMessageQueueId_t       radioToMainQueue, mainToRadioQueue;
static osTimerId_t       timerRadio;
extern ARM_DRIVER_I2C    Driver_I2C1;
static ARM_DRIVER_I2C*   I2Cdrv1 = &Driver_I2C1;
static uint16_t          RDA5807M_WriteReg[6];
static uint16_t          RDA5807M_ReadReg[6];
static uint16_t          RDA5807M_WriteRegDef[6] = {0xC004, 0x0000, 0x0100, 0x84D4, 0x4000, 0x0000};
static volatile uint32_t I2C_Event;
static uint8_t           registros_lectura[12];

void            ThreadRadio(void *argument);
static void     timer_callback(void);
int             I2C_Init(void);
static void     WriteAll(void);
static void     PowerOn(void);
static void     PowerOff(void);
static void     Frequency(uint32_t freq_kHz);
static void     SetVolume(uint8_t vol);
static void     SeekUp(void);
static void     SeekDown(void);
static void     I2C2_callback(uint32_t event);
static void     Readregisters(void);
static void     InitRegis(void);
static uint32_t seeFrec(void);
static uint32_t seePwr(void);

int Init_Radio(void) {
    radio_tid = osThreadNew(ThreadRadio, NULL, NULL);
    if (radio_tid == NULL) {
        return (-1);
    }

    osMessageQueueAttr_t attrs = { .name = "RadioQueue" };
    mainToRadioQueue = osMessageQueueNew(MAX_MSG, sizeof(radioMsg_t), &attrs);
    if (mainToRadioQueue == NULL) {
        return (-1);
    }

    timerRadio = osTimerNew((osTimerFunc_t)&timer_callback, osTimerPeriodic, NULL, NULL);
    if (timerRadio == NULL) {
        return (-1);
    }

    return (0);
}

void ThreadRadio(void *argument) {
    I2C_Init();
    InitRegis();
    WriteAll();
    RDA5807M_WriteReg[0] = RDA5807M_WriteReg[0] | RDA_RDS_ON;
    osTimerStart(timerRadio, 500U);

    radioMsg_t      msgMainRadio = {0};

    while (1) {
        osMessageQueueGet(mainToRadioQueue, &msgMainRadio, NULL, osWaitForever);
        switch (msgMainRadio) {
            case POWERON:
                PowerOn();
                Readregisters();
                break;
            case POWEROFF:
                PowerOff();
                Readregisters();
                break;
            case SEEKUP:
                SeekUp();
                Readregisters();
                break;
            case SEEKDOWN:
                SeekDown();
                Readregisters();
                break;
            default:
                Frequency(msgMainRadio);
                Readregisters();
                break;
        }
    }
}

int I2C_Init(void) {
    int error = 0;
    error |= I2Cdrv1->Initialize((ARM_I2C_SignalEvent_t)I2C2_callback);
    error |= I2Cdrv1->PowerControl(ARM_POWER_FULL);
    error |= I2Cdrv1->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
    error |= I2Cdrv1->Control(ARM_I2C_BUS_CLEAR, 0);
    return error;
}

void InitRegis(void) {
    for (int i = 0; i < 6; i++) {
        RDA5807M_WriteReg[i] = RDA5807M_WriteRegDef[i];
    }
    WriteAll();
}

void SeekUp(void) {
    RDA5807M_WriteReg[0] = RDA5807M_WriteReg[0] | RDA_SEEK_UP;
    WriteAll();
    RDA5807M_WriteReg[0] = RDA5807M_WriteReg[0] & RDA_SEEK_STOP;
}

void SeekDown() {
    RDA5807M_WriteReg[0] = RDA5807M_WriteReg[0] | RDA_SEEK_DOWN;
    WriteAll();
    RDA5807M_WriteReg[0] = RDA5807M_WriteReg[0] & RDA_SEEK_STOP;
}

void WriteAll(void) {
    uint8_t buffer[12];
    int     i, x = 0;
    for (i = 0; i < 12; i = i + 2) {
        buffer[i] = RDA5807M_WriteReg[x] >> 8;
        x++;
    }
    x = 0;
    for (i = 1; i < 13; i = i + 2) {
        buffer[i] = RDA5807M_WriteReg[x] & 0xFF;
        x++;
    }
    I2Cdrv1->MasterTransmit(0x10, buffer, 12, false);
    osThreadFlagsWait(RDA_TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
}

void PowerOn(void) {
    RDA5807M_WriteReg[1] = RDA5807M_WriteReg[1] | RDA_TUNE_ON;
    RDA5807M_WriteReg[0] = RDA5807M_WriteReg[0] | RDA_POWER;
    WriteAll();
    RDA5807M_WriteReg[1] = RDA5807M_WriteReg[1] & 0xFFEF;  // Disable tune after PowerOn operation
}

void PowerOff(void) {
    RDA5807M_WriteReg[0] = RDA5807M_WriteReg[0] ^ RDA_POWER;
    WriteAll();
}

void Frequency(uint32_t freq_kHz) {
    if (freq_kHz < 87000 || freq_kHz > 108000)
        return;

    //	float frecuencia = freq_kHz / 1000;									 // Pasa a Hz
    uint16_t Channel;
    Channel = ((((float)freq_kHz / 1000) - StartingFreq) / 0.1) + 0.05;
    Channel = Channel & 0x3FF;
    RDA5807M_WriteReg[1] = Channel * 64 + 0x10;  // Channel + TUNE-Bit + Band=00(87-108) + Space=00(100kHz)
    WriteAll();
    RDA5807M_WriteReg[1] = RDA5807M_WriteReg[1] & RDA_TUNE_OFF;
}

void SetVolume(uint8_t vol) {
    if (vol > 15)
        vol = 15;

    for (int j = 0; j < 6; j++)
        RDA5807M_WriteReg[j] = RDA5807M_WriteRegDef[j];

    if (vol != 0)
        PowerOn();
    else
        RDA5807M_WriteReg[0] = (RDA5807M_WriteReg[0] & 0xBFFF);  // Mute

    PowerOn();
    RDA5807M_WriteReg[3] = (RDA5807M_WriteReg[3] & 0xFFF0) | vol;  // Set New Volume
    WriteAll();
    RDA5807M_WriteReg[0] = RDA5807M_WriteReg[0] | RDA_RDS_ON;
}

void I2C2_callback(uint32_t event) {
    /* Save received events */
    I2C_Event |= event;

    if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
        /* Less data was transferred than requested */
        osThreadFlagsSet(radio_tid, RDA_TRANSFER_COMPLETE);
    }

    if (event == ARM_I2C_EVENT_TRANSFER_INCOMPLETE) {
        /* Transfer or receive is finished */
    }

    if (event == ARM_I2C_EVENT_ADDRESS_NACK) {
        /* Slave address was not acknowledged */
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

void timer_callback(void) {
    Readregisters();
}

void Readregisters(void) {
    uint8_t rcv[12];

    osDelay(50);
    I2Cdrv1->MasterReceive(0x10, rcv, 12, false);
    osThreadFlagsWait(RDA_TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
    for (int i = 0; i < 6; i++) {
        RDA5807M_ReadReg[i] = ((rcv[i * 2] << 8) | rcv[(i * 2) + 1]);
    }

    for (int i = 0; i < 12; i++) {
        registros_lectura[i] = rcv[i];
    }
}

uint32_t seeFrec(void) {
    Readregisters();
    return (((registros_lectura[0] & 0x03) << 8 | (registros_lectura[1] & 0xFF)) * 100) + 87000;
}

uint32_t seePwr(void) {
    Readregisters();
    return registros_lectura[3] << 1;
}
