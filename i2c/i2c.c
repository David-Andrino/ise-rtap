#include "i2c.h"

#include <Driver_I2C.h>
#include <cmsis_os2.h>

#define FLAGS_I2C_DONE 0x20

extern ARM_DRIVER_I2C    Driver_I2C1;
static ARM_DRIVER_I2C*   I2Cdrv1 = &Driver_I2C1;

static osMutexId_t i2cMutex;
static osThreadId_t blockerTid;
static void I2C1_callback(uint32_t event);

int Init_I2C(void) {
    i2cMutex = osMutexNew(NULL);
    if (i2cMutex == NULL) {
        return -1;
    }
    
    int error = 0;
    error |= I2Cdrv1->Initialize((ARM_I2C_SignalEvent_t)I2C1_callback);
    error |= I2Cdrv1->PowerControl(ARM_POWER_FULL);
    error |= I2Cdrv1->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
    error |= I2Cdrv1->Control(ARM_I2C_BUS_CLEAR, 0);
    return error;
}

void i2c_MasterTransmit(uint32_t addr,const uint8_t * data, uint32_t num, bool xfer_pending) {
    osMutexAcquire(i2cMutex, osWaitForever);
    blockerTid = osThreadGetId();
    I2Cdrv1->MasterTransmit(addr, data, num, xfer_pending);
    osThreadFlagsWait(FLAGS_I2C_DONE, osFlagsWaitAny, osWaitForever);
    osMutexRelease(i2cMutex);
}

void i2c_MasterReceive(uint32_t addr, uint8_t * data, uint32_t num, bool xfer_pending) {
    osMutexAcquire(i2cMutex, osWaitForever);
    blockerTid = osThreadGetId();
    I2Cdrv1->MasterReceive(addr, data, num, xfer_pending);
    osThreadFlagsWait(FLAGS_I2C_DONE, osFlagsWaitAny, osWaitForever);
    osMutexRelease(i2cMutex);
}

static void I2C1_callback(uint32_t event) {

    if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
        /* Less data was transferred than requested */
        osThreadFlagsSet(blockerTid, FLAGS_I2C_DONE);
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