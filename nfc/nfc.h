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
 *
 */

#include <stdint.h>
#include <cmsis_os2.h>

/**
 * Inicializa el modulo NFC.
 *
 * @return 0 si se ha realizado correctamente. Otro valor si no.
 **/
int Init_NFC(void);

#endif
