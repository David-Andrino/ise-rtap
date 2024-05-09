#ifndef _GUI
#define _GUI

#include "gui.h"
#include "lvgl.h"
#include "touchpad.h"
#include "tft.h"
#include "cmsis_os2.h"
#include "../Control/controlThread.h"
/**
 * @brief Enumeración de mensajes de salida al LCD
 */

extern osMessageQueueId_t lcdQueue;

typedef enum {
    LCD_OUT_VOL,            /*< Cambio de volumen. Contenido es el volumen [0, 10]*/
    LCD_OUT_BANDS,          /*< Cambio de filtro. Contenido es primer byte la banda [0,4] segundo la cantidad [-9, 9] */
    LCD_OUT_INPUT_SEL,      /*< Cambio de entrada. Contenido es 0 para la radio y 1 para MP3 */
    LCD_OUT_OUTPUT_SEL,     /*< Cambio de salida. Contenido es 0 para cascos y 1 para altavoz */
    LCD_OUT_CONS,           /*< Medida de consumo. Contenido es mA en [0, 2000] */
    LCD_OUT_RADIO_FREQ,     /*< Cambio de frecuencia de la radio. Contenido es la frecuencia en centenas de kHz */
} lcd_out_msg_type_t;

/**
 * @brief Estructura para los mensajes de salida del LCD
 */
typedef struct {
    lcd_out_msg_type_t type; 
    uint16_t payload;
} lcd_out_msg_t;

int Init_Threads_LCD (void);
void get_cadena(char *buf, float freq);
void EnterStandbyMode(void);
#endif