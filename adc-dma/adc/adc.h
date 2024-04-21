#ifndef ADC_H
#define ADC_H
#include <stdint.h>

int  sampling_init(void);
int  sampling_start(uint16_t* buffer, uint32_t bufSize);
void sampling_stop(void);
    
#endif