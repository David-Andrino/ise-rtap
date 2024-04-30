#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "os_tick.h"

#include "gui_threads.h"
#include "Board_LED.h"
#include "stdio.h"
#include "eqCoefficients.h"
#define INIT 1

osThreadId_t tid_Thread_tasks, tid_Thread_time, tid_Thread_stuff;
 
void Thread_tasks (void *argument);                   // thread function
void Thread_time (void *argument);
void Thread_stuff (void *args);

static osMutexId_t mut;

extern lv_display_t * disp;

//char padres[130000] = {0};  // Ocupa y no hace nada

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
	
	int i = 0, j = 0;
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

typedef struct{
	char  cadena[50];
	float freq;
} cadena_t;

const cadena_t cadenas[] = {{"Onda Latina", 87.6} , {"Europa FM", 87.7} , {"Europa FM", 87.8} , {"Hit FM", 87.9} , {"La Nueva FM 87.9", 87.9} , {"Ecuatoriana FM", 88.4} , {"COPE de la Sierra", 88.7} ,
											{"La Kalle", 88.8} , {"Los 40 Classic", 89} , {"Antena Cemu Radio", 89.2} , {"esRadio", 89.2} , {"Radio Aranjuez", 89.3} , {"Onda Fuenlabrada", 89.4} , {"SER Madrid Norte", 89.6} , 
											{"Radio Maria", 89.7} , {"Hit FM", 89.9} , {"Hit FM", 90} , {"Super Latina", 90.1} , {"Canela MD Radio", 90.5} , {"Onda Cero Aranjuez", 90.7} , {"Radio Maria", 90.7} , {"Imperia FM", 90.8} ,
											{"Europa FM", 91} , {"Loca Urban", 91.2} , {"Onda del Plata", 91.2} , {"Cadena 100 Sierra", 91.3} , {"Onda Cero Cienpozuelos", 91.4} , {"Radio Tentacion", 91.4} , {"Radio Universidad de Alcala", 91.5} ,
											{"Cadena Dial Madrid", 91.7} , {"Pozuelo Radio", 91.9} , {"Radio Activa Europa", 91.9} , {"COPE + Madrid", 92} , {"Cadena Dial Madrid Sur", 92.1} , {"Los 40 Sierra", 92.1} , {"COPE Pinares", 92.2} , 
											{"Los 40 Dance", 92.4} , {"Radio Solidaria", 92.6} , {"Rock FM", 92.7} , {"Hit FM", 92.8} , {"China FM", 92.9} , {"La 93", 93} , {"Radio Inter", 93.5} , {"Autocine Madrid", 93.7} , 
											{"Master FM", 93.7} , {"Los 40 Madrid", 93.9} , {"Radio Marca", 94.2} , {"SER Madrid Sur", 94.4} , {"Esencia Radio", 94.5} , {"Gozadera FM", 94.5} , {"Melodia FM", 94.6} , {"COPE + Madrid", 94.8} , 
											{"Radio Intereconomia", 95.1} , {"Bless FM", 95.3} , {"Espacio 4 FM", 95.4} , {"Radio Intercontinental", 95.4} , {"SER Madrid Sierra", 95.4} , {"RNE Radio 3", 95.8} , {"Loca FM", 96} , 
											{"Cadena Dial Sierra", 96.2} , {"La Suegra FM", 96.7} , {"Los 40 Urban Sierra", 96.9} , {"Play FM", 97} , {"Radio Maria", 97.2} , {"EDM Radio", 97.4} , {"COPE + Madrid", 97.5} , 
											{"Onda Cero Alcala", 97.6} , {"Los 40 Madrid Sur", 97.7} , {"La Mega Estacion", 98.2} , {"Onda Diamante", 98.3} , {"Melodia FM", 98.4}  , {"Hit FM", 98.5} , {"RNE Radio Clasica", 98.8} , 
											{"Canela FM Radio", 99.3} , {"Globo FM", 99.3} , {"Cadena 100 Madrid", 99.5} , {"Casi 100 FM", 99.9} , {"Onda Cero Arganda", 100} , {"Onda Cero Colmenar", 100.1} , {"Onda Cero Torrejon", 100.2} , 
											{"Megastar FM", 100.3} , {"Radio Maria", 100.3} , {"Vaughan Radio", 100.4} , {"Pepe Radio", 100.5} , {"Megastar FM", 100.7} , {"Kiss FM", 100.9} , {"Megastar FM", 100.9} , {"Starlite Radio", 100.9} , 
											{"COPE + Madrid", 101} , {"Da la Palabra Radio", 101.1} , {"Radio Solidaria", 101.5} , {"Rock FM", 101.6} , {"Rock FM", 101.7} , {"Kandela Estereo", 101.9} , {"Onda Cero Coslada", 102} , 
											{"Decision Radio", 102.1} , {"Cadena Dial Madrid Norte", 102.2} , {"Dos.Dos Radio", 102.2} , {"Hit FM", 102.2} , {"Radio Shekina", 102.2} , {"SER Madrid Oeste", 102.3} , {"Factory FM", 102.4} , 
											{"Kiss FM", 102.7} , {"Unika FM", 103} , {"SER Henares", 103.1} , {"Capital Radio", 103.2} , {"Free FM Classic", 103.3} , {"Kiss FM", 103.4} , {"Radio Marca", 103.5} , {"Los 40 Urban", 103.9} , 
											{"Onda Davinci", 104} , {"SER+ Madrid", 104.3} , {"Informa Radio", 104.5} , {"Melodia FM", 104.6} , {"A Viva Voz", 104.7} , {"Ibiza BPM Radio", 104.7} , {"RNE", 104.9} , 
											{"Bom Radio", 105.1} , {"COPE + Madrid", 105.1} , {"Motiva", 106.5} , {"Hit FM", 106.6} , {"Onda Cero El Escorial Sierra", 106.6} , {"Radio Almenara", 106.7} , {"Melodia FM", 106.8} , 
											{"Radio El Alamo", 106.8} , {"Radio Chinchon", 107} , {"Radio Libertad", 107} , {"Radio Cultural La Oliva", 107.1} , {"Radio Miraflores", 107.1} , {"Rec Radio", 107.1} , {"Rio FM", 107.2} , 
											{"Torre FM Onda Torre", 107.2} , {"Dynamis Radio", 107.3} , {"Onda Merlin Comunitaria", 107.3} , {"Onda Pedriza", 107.3} , {"Radio Madrid Sierra", 107.3} , {"Radio 21", 107.3} , {"Radio 21", 107.3} , 
											{"Radio Matorral", 107.4} , {"Radio Villalba", 107.4} , {"Radio Complutense", 107.5} , {"Radio Enlace", 107.5} , {"Radio Nuevo Baztan", 107.5} , {"Radio Vallekas", 107.5} , {"7 FM", 107.5} , 
											{"Radio Corazon Tropical", 107.6} , {"Radio Morata", 107.6} , {"Radio Romanul", 107.7} , {"Onda Aranjuez", 107.8} , {"Radio Braojos", 107.8} , {"Radio Estremera", 107.8} , {"Radio Union", 107.8} , 
											{"Radio Utopia", 107.8} , {"Radio 21", 107.8} , {"Onda Verde", 107.9} , {"Radio Soto", 107.9} , {"Radio Viator", 107.9} , {"Radio 21", 107.9} , {"Madrid 24 Horas Radio", 108} , 
											{"Exito Radio", 100.0 + 106.5 + 106.6 + 107.4} , {"Onda Madrid", 101.3 + 106.0} , {"RNE", 88.2} , {"RNE Radio 5 TN", 657 + 90.3} , {"Radio Madrid", 810 + 105.4} , 
											{"Dynamis Radio", 87.5 + 98.6} , {"Ushuaia Radio", 87.7 + 94.9} , {"Ucrania FM", 88.6 + 89.7} , {"Pepe Radio", 89.3 + 96.2} , {"Radio Maria", 90.7 + 96.9} , {"Ecuashyri", 91.9 + 103.7} , 
											{"Vida FM", 92.2 + 94.2 + 104.1} , {"RNE Radio 3", 93.2} , {"Onda Cero Madrid", 954 + 98.0} , {"RNE Radio Clasica", 96.5} , {"Free FM Rock", 97.4 + 97.8} , {"esRadio", 99.1}, {"esRadio", 105.7} , 
											{"Sol Radio", 99.8}, {"Sol Radio", 107.1} , {"COPE Madrid", 106.3}};

void get_cadena(char *buf, float freq){
	for (int index = 0; index < 150; index++){
		if(freq == cadenas[index].freq) {
			sprintf(buf, "%s", cadenas[index].cadena);
		}
	}
}
