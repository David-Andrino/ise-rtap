#ifndef DSP_H
#define DSP_H

#include <stdint.h>

#define DSP_BANDS 5
#define DSP_NUM_STAGES 2
#define DSP_LEVELS 5
#define DSP_POST_SHIFT 2
#define DSP_BUF_SIZE 2048

typedef uint8_t dsp_coefs_t[DSP_BANDS];

void dsp_init(dsp_coefs_t levels);
void dsp_change_coefficients(dsp_coefs_t* coefs);
void dsp_process_buffer(uint16_t* in, uint16_t* out, uint32_t bufSize); 

#endif
