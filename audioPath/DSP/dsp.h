#ifndef DSP_H
#define DSP_H

#include <stdint.h> 

#define DSP_BUFSIZE 4096

extern int dsp_tim_init(void);
extern void processSamples(uint16_t* in, uint16_t* out);

#endif
