#ifndef RTC_H
#define RTC_H
#include "stm32f7xx_hal.h"

typedef struct {
    uint64_t weekday: 8;
    uint64_t year:    7;
    uint64_t month:   4;
    uint64_t day:     5;
    uint64_t hour:    5;
    uint64_t min:     6;
    uint64_t sec:     6;
} rtc_time_t;

extern int RTC_init(void (*alarmACallback));
extern int RTC_readTime(rtc_time_t* time);
extern int RTC_setHour(int hour, int min, int sec);
extern int RTC_setDate(int day, int month, int year);
extern int RTC_setTime(rtc_time_t* time);
extern int RTC_activateAlarm(uint32_t alarm, void (*callback)(void));
extern int RTC_deactivateAlarm(uint32_t alarm);

#endif
