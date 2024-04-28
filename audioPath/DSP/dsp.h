#ifndef DSP_H
#define DSP_H

#include <stdint.h>

extern int  dsp_init(void);
extern void processSamples(uint16_t* in, uint16_t* out);
extern void dsp_configure_filters(int8_t* bands, uint8_t vol);

#endif
