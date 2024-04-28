#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "os_tick.h"

#include "gui_threads.h"

#include "Board_LED.h"
#define INIT 1

osThreadId_t tid_Thread_tasks, tid_Thread_time, tid_Thread_leds;                        // thread id
 
void Thread_tasks (void *argument);                   // thread function
void Thread_time (void *argument);
void Thread_Leds (void *args);

static osMutexId_t mut;

extern lv_display_t * disp;

int Init_Thread (void) {
 
	osThreadAttr_t att_task = {
		.name = "LVGL Tasks",
		.stack_size = 12400,
		.priority = osPriorityNormal
	};
	
	osThreadAttr_t att_time = {
		.name = "LVGL Times",
		.stack_size = 12400,
		.priority = osPriorityNormal
	};
	
	osMutexAttr_t att_mut = {
		.name = "LVGL Mutex",
		.cb_size = 150000
	};
	
	mut = osMutexNew(&att_mut);
	
  tid_Thread_tasks = osThreadNew(Thread_tasks, NULL, &att_task);
	tid_Thread_time = osThreadNew(Thread_time, NULL, &att_time);
	tid_Thread_leds = osThreadNew(Thread_Leds, NULL, NULL);
	
	if (tid_Thread_tasks == NULL) {
    return(-1);
  }
 
  return(0);
}
void Thread_Leds(void *args){
	LED_Initialize();
	
	int i = 0;
  while (1) {
    LED_SetOut(++i);
    osDelay(500);                            // suspend thread
  }
}
 
void Thread_time(void *arg){
	osThreadFlagsWait(INIT, osFlagsWaitAll, osWaitForever);
	
	while (1) {
		osMutexAcquire(mut, osWaitForever);
		lv_tick_inc(5);
    osMutexRelease(mut);
		osDelay(5);
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
