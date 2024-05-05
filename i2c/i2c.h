/**
 * @file i2c.h
 *
 * @brief Módulo de acceso concurrente al periférico I2C
 *
 * @author Ruben Agustin
 * @author David Andrino
 * @author Estela Mora
 * @author Fernando Sanz
 *
 * Mediante un candado, ofrece protección contra acceso concurrente al 
 * periférico I2C.
 *
 */
#ifndef I2C_GUARD_H
#define I2C_GUARD_H

#include <cmsis_os2.h>
#include <stdbool.h>

/**
 * Inicializa el modulo del I2C. Debe llamarse antes de inicializar los 
 * módulos que lo utilizan.
 *
 * @return int 0 si se ha realizado correctamente. Otro valor si no.
 **/
int Init_I2C(void);

/**
 * Realiza una escritura I2C. Ofrece una interfaz idéntica a la función homónima de CMSIS Driver
 *
 * @param addr Dirección del esclavo I2C
 * @param data Puntero al buffer con los datos a escribir
 * @param num  Número de bytes a escribir
 * @param xfer_pending Booleano que indica si se debe dejar la transferencia sin finalizar
 */
void i2c_MasterTransmit(uint32_t addr,const uint8_t * data, uint32_t num, bool xfer_pending);

/**
 * Realiza una lectura I2C. Ofrece una interfaz idéntica a la función homónima de CMSIS Driver
 *
 * @param addr Dirección del esclavo I2C
 * @param data Puntero al buffer donde escribir los datos leidos
 * @param num  Número de bytes a leer
 * @param xfer_pending Booleano que indica si se debe dejar la transferencia sin finalizar
 */
void i2c_MasterReceive(uint32_t addr, uint8_t * data, uint32_t num, bool xfer_pending);

#endif
