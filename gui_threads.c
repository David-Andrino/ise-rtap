#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "os_tick.h"

#include "gui_threads.h"

#include "Board_LED.h"
#include "eqCoefficients.h"
#define INIT 1

osThreadId_t tid_Thread_tasks, tid_Thread_time, tid_Thread_stuff;                        // thread id
 
void Thread_tasks (void *argument);                   // thread function
void Thread_time (void *argument);
void Thread_stuff (void *args);

static osMutexId_t mut;

extern lv_display_t * disp;

int Init_Thread (void) {
 
	osThreadAttr_t att_task = {
		.name = "LVGL Tasks",
		.stack_size = 12400,
		.priority = osPriorityHigh
	};
	
	osThreadAttr_t att_time = {
		.name = "LVGL Times",
		.stack_size = 12400,
		.priority = osPriorityHigh
	};
	
	osMutexAttr_t att_mut = {
		.name = "LVGL Mutex",
		.cb_size = 150000
	};
	
	mut = osMutexNew(&att_mut);
	
  tid_Thread_tasks = osThreadNew(Thread_tasks, NULL, &att_task);
	tid_Thread_time = osThreadNew(Thread_time, NULL, &att_time);
	tid_Thread_stuff = osThreadNew(Thread_stuff, NULL, NULL);
	
	if (tid_Thread_tasks == NULL) {
    return(-1);
  }
 
  return(0);
}
void Thread_stuff (void *args){
	LED_Initialize();
	
	int i = 0;
	int data[2];
  while (1) {
		data[0] = i%950; data[1] = coeffTable[i%950];
    LED_SetOut(++i);
		lv_async_call(async_cb, data);
		lv_async_call(consumo_async_cb, data);
    osDelay(500);
  }
}
 
void Thread_time(void *arg){
	osThreadFlagsWait(INIT, osFlagsWaitAll, osWaitForever);
	
	while (1) {
		osMutexAcquire(mut, osWaitForever);
		lv_tick_inc(3);
    osMutexRelease(mut);
		osDelay(3);
  }
}

void Thread_tasks (void *argument) {
//	lv_tick_set_cb(OS_Tick_GetCount);
//	lv_tick_set_cb(osKernelGetTickCount);
	
//	lv_demo_widgets();
	
	lv_init();
	
	tft_init();
	touchpad_init();	
	
	lv_gui();
	
	osThreadFlagsSet(tid_Thread_time, INIT);
	
	uint32_t ms;
  while (1) {
		osMutexAcquire(mut, osWaitForever);
		ms = lv_task_handler(); // devuelve el tiempo que necesita hasta la proxima llamada
		osMutexRelease(mut);
    osDelay(ms);
  }
}
