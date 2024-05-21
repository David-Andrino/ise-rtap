#ifndef NFC_H
#define NFC_H

/**
 * @file
 *
 * @brief Modulo NFC
 *
 * @author Ruben Agustin
 * @author David Andrino
 * @author Estela Mora
 * @author Fernando Sanz
 *
 * Utilizando el perif�rico I2C1, lee el contenido NDEF de la Dynamic Tag M24SR 
 * y se lo env�a al hilo principal. Lee si es una canci�n o una emisora 
 * de radio y cu�l es.
 * La direccion I2C del NFC es 0x56
 * 
 * GPO: A nivel alto cuando no hay comunicación en marcha
 * DisableRF: Cuando esta a nivel alto deshabilita la comunicación RF.
 */

#include <stdint.h>
#include <cmsis_os2.h>

#define GPO_GPIO_PIN  GPIO_PIN_12
#define GPO_GPIO_PORT GPIOA

#define DIS_GPIO_PIN  GPIO_PIN_14
#define DIS_GPIO_PORT GPIOB

#define DIS_GPIO_ON  GPIO_PIN_SET
#define DIS_GPIO_OFF GPIO_PIN_RESET

/**
 * Inicializa el modulo NFC.
 *
 * @return 0 si se ha realizado correctamente. Otro valor si no.
 **/
int Init_NFC(void);

#endif
