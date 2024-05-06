#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "rtcThread.h"
#include "rtc.h"
#include "../Control/controlThread.h"

#include <rl_net.h>
#include <stdio.h>
#include <time.h>

#define MAX_MSG 8
#define RTC_SEC_FLAG 0x01
#define RTC_NTP_FLAG 0x02

const NET_ADDR4 sntp_servers[3] = { 
    {NET_ADDR_IP4, 123, 82,  223, 203, 159}, // 0.pool.ntp.org
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

uint32_t cnt = 0;

static void alarmACallback(void);
static void timerOnceCallback(void* arg);
static void timerPeriodicCallback(void* arg);
static void ntpCallback(uint32_t time, uint32_t subsec);

static msg_ctrl_t msgToMain = {
	.type = MSG_RTC
};

int RTC_thread_init() {
    rtc_tid = osThreadNew((osThreadFunc_t) RTC_thread_main, NULL, NULL);
    if (rtc_tid == NULL) {
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
            msgToMain.rtc_msg.day = currentTime.day;
            msgToMain.rtc_msg.month = currentTime.month;
            msgToMain.rtc_msg.year = currentTime.year;
            msgToMain.rtc_msg.hour = currentTime.hour;
            msgToMain.rtc_msg.minute = currentTime.min;
            msgToMain.rtc_msg.second = currentTime.sec;
			osMessageQueuePut(ctrl_in_queue, &msgToMain, NULL, osWaitForever);
            osMutexRelease(rtc_mutex);
        }
        if (flag & RTC_NTP_FLAG) {
            struct tm* utcTime = localtime(&ntp_time);
            if (utcTime->tm_mon > 2 && (utcTime->tm_mon < 9 || (utcTime->tm_mon == 9 && utcTime->tm_mday < 27))) {
                ntp_time += 3600; // UTC +2
                utcTime = localtime(&ntp_time);
            }
                
            RTC_setDate(utcTime->tm_mday, utcTime->tm_mon + 1, utcTime->tm_year - 100);
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