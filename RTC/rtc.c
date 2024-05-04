#include "rtc.h"
#include "../main.h"

static RTC_HandleTypeDef hrtc;
void (*alarmACallback)(void);
void (*alarmBCallback)(void);

void RTC_Alarm_IRQHandler() {
    HAL_RTC_AlarmIRQHandler(&hrtc);
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *rtcHandle) {
    alarmACallback();
}

void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *rtcHandle) {
    alarmBCallback();
}

int RTC_init(void (*alarmACallback)) {
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_RTC_ENABLE();
    
    hrtc.Instance = RTC; 
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 249;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    __HAL_RTC_RESET_HANDLE_STATE(&hrtc);
    
    if (HAL_RTC_Init(&hrtc) != HAL_OK) return -1;
    
    __HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
    if (HAL_RTC_WaitForSynchro(&hrtc) != HAL_OK) return -1;
    __HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);
    
    int ret = 0;
    if (alarmACallback != NULL) {
        RTC_activateAlarm(RTC_ALARM_A, alarmACallback);
    }
    
    return ret;
}

int RTC_readTime(rtc_time_t* time) {
    RTC_DateTypeDef fecha;
    RTC_TimeTypeDef hora;
    
    int ret = 0;
    ret |= HAL_RTC_GetTime(&hrtc, &hora, RTC_FORMAT_BIN)  != HAL_OK;
    ret |= HAL_RTC_GetDate(&hrtc, &fecha, RTC_FORMAT_BIN) != HAL_OK;
    
    time->year    = fecha.Year;
    time->weekday = fecha.WeekDay;
    time->month   = fecha.Month;
    time->day     = fecha.Date;
    
    time->hour = hora.Hours;
    time->min = hora.Minutes;
    time->sec = hora.Seconds;
    
    return ret;
}

int RTC_setHour(int hour, int min, int sec) {
    if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return -1;
    }
    
    RTC_TimeTypeDef hora = {
        .Hours   = hour,
        .Minutes = min,
        .Seconds = sec
    };
    return HAL_RTC_SetTime(&hrtc, &hora, RTC_FORMAT_BIN);
}

int RTC_setDate(int day, int month, int year) {
    if (day < 1 || day > 31 || month < 1 || month > 12 || year < 0 || year > 99) {
        return -1;
    }
    
    RTC_DateTypeDef fecha = {
        .Date = day,
        .Month = month,
        .Year = year
    };
    return HAL_RTC_SetDate(&hrtc, &fecha, RTC_FORMAT_BIN);
}

int RTC_setTime(rtc_time_t* time) {
    return RTC_setDate(time->day, time->month, time->year) | RTC_setHour(time->hour, time->min, time->sec);
}

int RTC_activateAlarm(uint32_t alarm, void (*callback)(void)) {
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
    RTC_AlarmTypeDef alarmStruct = { 0 };
    
    alarmStruct.Alarm = alarm;
    if (alarm == RTC_ALARM_A) {
        alarmStruct.AlarmMask = RTC_ALARMMASK_ALL;
        alarmStruct.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;
        alarmACallback = callback;
    } else {
        alarmStruct.AlarmTime.Seconds = 0;
        alarmStruct.AlarmTime.SubSeconds = 00;
        alarmStruct.AlarmMask = RTC_ALARMMASK_ALL ^ RTC_ALARMMASK_SECONDS;// RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES;
        alarmStruct.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;
        alarmBCallback = callback;
    }
    
    return HAL_RTC_SetAlarm_IT(&hrtc, &alarmStruct, RTC_FORMAT_BIN);
}
int RTC_deactivateAlarm(uint32_t alarm) {
    HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);
    return HAL_RTC_DeactivateAlarm(&hrtc, alarm);
}