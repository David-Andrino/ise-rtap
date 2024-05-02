#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "rtcThread.h"
#include "rtc.h"

#include <rl_net.h>
#include <stdio.h>
#include <time.h>

#define MAX_MSG 8
#define RTC_SEC_FLAG 0x01
#define RTC_NTP_FLAG 0x02

const NET_ADDR4 sntp_servers[3] = { // 03/02/2024 7:04 PM
    {NET_ADDR_IP4, 123, 185,  90, 148, 209}, // 0.pool.ntp.org
    {NET_ADDR_IP4, 123, 208,  85,  20, 220}, // 1.pool.ntp.org
    {NET_ADDR_IP4, 123, 158, 227,  98,  15}, // 2.pool.ntp.org    
};

static osThreadId_t rtc_tid;
static osMutexId_t  rtc_mutex;
static osTimerId_t  rtc_timer_once, rtc_timer_periodic;
static volatile uint8_t rtc_lcd_enable = 1;
static rtc_time_t currentTime = { 0 };
static const int selectedServer = 0;
static time_t ntp_time = 0;

osMessageQueueId_t  rtc_queue;
uint32_t cnt = 0;

static void alarmACallback(void);
static void timerOnceCallback(void* arg);
static void timerPeriodicCallback(void* arg);
static void ntpCallback(uint32_t time, uint32_t subsec);

int RTC_thread_init() {
    rtc_tid = osThreadNew((osThreadFunc_t) RTC_thread_main, NULL, NULL);
    if (rtc_tid == NULL) {
        return -1;
    }
    
    rtc_queue = osMessageQueueNew(MAX_MSG, sizeof(rtc_time_t), NULL);
    if (rtc_queue == NULL) {
        return -1;
    }
    
    rtc_mutex = osMutexNew(NULL);
    if (rtc_mutex == NULL) {
        return -1;
    }
    
    rtc_timer_once = osTimerNew(timerOnceCallback, osTimerOnce, NULL, NULL);
    if (rtc_timer_once == NULL) {
        return -1;
    }
    
    rtc_timer_periodic = osTimerNew(timerPeriodicCallback, osTimerPeriodic, NULL, NULL);
    if (rtc_timer_periodic == NULL) {
        return -1;
    }
    
    if (RTC_init(alarmACallback) != 0) {
        return -1;
    }
    
    
    return 0;
}

__NO_RETURN int RTC_thread_main(void* args) {
    (void) args;
    
    osTimerStart(rtc_timer_once, 1000); // 1 SEC
    
    while (1) {
        uint32_t flag = osThreadFlagsWait(RTC_SEC_FLAG | RTC_NTP_FLAG, osFlagsWaitAny, osWaitForever);
        if (flag & RTC_SEC_FLAG) {
            osMutexAcquire(rtc_mutex, osWaitForever);
            RTC_readTime(&currentTime);
						// Poner hora en la cola
            osMutexRelease(rtc_mutex);
        }
        if (flag & RTC_NTP_FLAG) {
            struct tm* utcTime = localtime(&ntp_time);
            if (utcTime->tm_mon > 2 && (utcTime->tm_mon < 9 || (utcTime->tm_mon == 9 && utcTime->tm_mday < 27))) {
                ntp_time += 3600; // UTC +2
                utcTime = localtime(&ntp_time);
            }
                
            RTC_setDate(utcTime->tm_mday + 1, utcTime->tm_mon + 1, utcTime->tm_year - 100);
            RTC_setHour(utcTime->tm_hour, utcTime->tm_min, utcTime->tm_sec);
        }
    }
}

void alarmACallback() {
    osThreadFlagsSet(rtc_tid, RTC_SEC_FLAG);
		/*------------ CAMBIAR ESTO --------------------*/
}

void timerOnceCallback(void* arg) {
    timerPeriodicCallback(NULL);
    osTimerStart(rtc_timer_periodic, 180000); // 3 min
}
void timerPeriodicCallback(void* arg) {
    netSNTPc_GetTime((NET_ADDR*)&sntp_servers[selectedServer], ntpCallback);
}
void ntpCallback(uint32_t time, uint32_t subsec) {
    if (time > 0) {
        ntp_time = time + 3600; // UTC +1
        osThreadFlagsSet(rtc_tid, RTC_NTP_FLAG);
    }
}