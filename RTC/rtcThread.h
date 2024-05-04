#ifndef RTC_THREAD_H
#define RTC_THREAD_H

#include <cmsis_os2.h>
#include "stm32f7xx_hal.h"
#include <rl_net.h>

extern osMessageQueueId_t rtc_queue;

extern int  RTC_thread_init(void);
extern int  RTC_thread_main(void*);
extern void RTC_setLCDEnable(uint8_t enable);
extern uint32_t RTC_printFormattedHour(char* dst, const char* fmt);
extern uint32_t RTC_printFormattedDate(char* dst, const char* fmt);

#endif
