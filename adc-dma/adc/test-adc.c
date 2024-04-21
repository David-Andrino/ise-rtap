#include "test-adc.h"
#include "adc.h"
#include "../main.h"

#include <string.h>
#include <stdio.h>

static int done = 0;
static uint16_t  buffer[2048];
static uint16_t* half_buf = &buffer[1024];

static void samplingFullCb();

void test_adc() {
  int ret = sampling_init(NULL, samplingFullCb);
  sampling_start(buffer, 2048);
  
  while (!done);

  char line[156] = {0};
  for (int i = 0; i < 50; i++) {
      for (int j = 0; j < 32; j++) {
        snprintf(line + 5*j, 6, "%04.4X ", buffer[32*i + j]);
      }
      printf("%s\n", line);
      HAL_Delay(100);
  }
}

void samplingFullCb() {
    done = 1;
    sampling_stop();
}