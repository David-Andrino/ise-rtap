#ifndef I2C_GUARD_H
#define I2C_GUARD_H

#include <cmsis_os2.h>
#include <stdbool.h>

int Init_I2C(void);

void i2c_MasterTransmit(uint32_t addr,const uint8_t * data, uint32_t num, bool xfer_pending);

void i2c_MasterReceive(uint32_t addr, uint8_t * data, uint32_t num, bool xfer_pending);

#endif
