#ifndef DSP_H
#define DSP_H

#include <stdint.h>

extern int  dsp_init(void);
extern void processSamples(uint16_t* in, uint16_t* out);

#endif
