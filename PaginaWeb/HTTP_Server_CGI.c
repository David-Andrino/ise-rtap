/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network:Service
 * Copyright (c) 2004-2018 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server_CGI.c
 * Purpose: HTTP Server CGI Module
 * Rev.:    V6.0.0
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE
#include "controlThread.h"
#include <stdlib.h>
#include "ThreadWeb.h"

#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic push
#pragma  clang diagnostic ignored "-Wformat-nonliteral"
#endif



//// Local variables.
static uint8_t ip_addr[NET_ADDR_IP6_LEN];
static char    ip_string[40];
static msg_ctrl_t msgToMain = {
	.type = MSG_WEB
};

// My structure of CGI status variable.
typedef struct {
  uint8_t idx;
  uint8_t unused[3];
} MY_BUF;
#define MYBUF(p)        ((MY_BUF *)p)


char canciones [25][30] = {
	"Cancion 1",
	"Cancion 2",
	"Cancion 3",
	"Cancion 4",
	"Cancion 5"
};
uint8_t song_cnt = 5;

options opciones = {0};

static void sendToQueue(web_msg_type_t type, uint16_t payload);
static void searchSong(char* name);

// Process query string received by GET request.
void netCGI_ProcessQuery (const char *qstr) {
  netIF_Option opt = netIF_OptionMAC_Address;
  int16_t      typ = 0;
  char var[40];

  do {
    // Loop through all the parameters
    qstr = netCGI_GetEnvVar (qstr, var, sizeof (var));
    // Check return string, 'qstr' now points to the next parameter

    switch (var[0]) {
      case 'i': // Local IP address
        opt = netIF_OptionIP4_Address;
        
        break;

      case 'm': // Local network mask
        opt = netIF_OptionIP4_SubnetMask;
        break;

      case 'g': // Default gateway IP address
        if (var[1] == '4') { opt = netIF_OptionIP6_DefaultGateway; }
        break;

      case 'p': // Primary DNS server IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_PrimaryDNS; }
        break;

      case 's': // Secondary DNS server IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_SecondaryDNS; }
        break;
      
      default: var[0] = '\0'; break;
    }

    switch (var[1]) {
      case '4': typ = NET_ADDR_IP4; break;
      default: var[0] = '\0'; break;
    }

    if ((var[0] != '\0') && (var[2] == '=')) {
      netIP_aton (&var[3], typ, ip_addr);
      // Set required option
      netIF_SetOption (NET_IF_CLASS_ETH, opt, ip_addr, sizeof(ip_addr));
    }
  } while (qstr);
}

// Process data received by POST request.
// Type code: - 0 = www-url-encoded form data.
//            - 1 = filename for file upload (null-terminated string).
//            - 2 = file upload raw data.
//            - 3 = end of file upload (file close requested).
//            - 4 = any XML encoded POST data (single or last stream).
//            - 5 = the same as 4, but with more XML data to follow.
void netCGI_ProcessData (uint8_t code, const char *data, uint32_t len) {
  char var[40],passw[12];

  if (code != 0) {
    // Ignore all other codes
    return;
  }
	
	  if (len == 0) {
    return;
  }
  passw[0] = 1;
  do {
    // Parse all parameters
    data = netCGI_GetEnvVar (data, var, sizeof (var));
    if (var[0] != 0) {
      // First character is non-null, string exists
			if (strncmp(var, "entrada=",8) == 0){
				if (strncmp(var, "entrada=radio",13) == 0){
					opciones.entrada = RADIO;
					sendToQueue(WEB_INPUT_SEL, 0);
				} 
				else if (strncmp(var, "entrada=mp3",11) == 0){
					opciones.entrada = MP3;
					sendToQueue(WEB_INPUT_SEL, 1);
				}
			}
			else if (strncmp(var, "salida=",7) == 0){
				if (strncmp(var, "salida=altavoz",14) == 0){
					opciones.salida = ALTAVOZ;
					sendToQueue(WEB_OUTPUT_SEL, 1);
				} 
				else if (strncmp(var, "salida=cascos",13) == 0){
					opciones.salida = AURICULARES;
					sendToQueue(WEB_OUTPUT_SEL, 0);
				}
			}
			else if (strncmp(var, "consumo=",8) == 0){
				opciones.bajo_consumo = 1;
				sendToQueue(WEB_LOW_POWER,0);
			}
			else if (strncmp(var, "frec_sint=",10) == 0){
				float tmp;
				sscanf(var, "frec_sint=%f", &tmp);
				opciones.freq_actual = (int)(tmp*10);
				sendToQueue(WEB_RADIO_FREQ, opciones.freq_actual);
			}
			else if (strncmp(var, "seekup=",7) == 0){
				sendToQueue(WEB_SEEK, 1);
			}
			else if (strncmp(var, "seekdown=",9) == 0){
				sendToQueue(WEB_SEEK, 0);
			}
			else if (strncmp(var, "mute=",5) == 0){
				opciones.mute =! opciones.mute;
				opciones.vol = (opciones.mute == 1 ? 0 : opciones.prev_vol);
				sendToQueue(WEB_VOL, opciones.vol);
			}
			else if (strncmp(var, "formatted_vol=",14) == 0){
				sscanf(var, "formatted_vol=%hhd", &opciones.vol);
				opciones.prev_vol = opciones.vol;
				sendToQueue(WEB_VOL, opciones.vol);
			}
			else if (strncmp(var, "save_conf.x=",12) == 0){
				sendToQueue(WEB_SAVE_SD, 0);
			}
			else if (strncmp(var, "eq1=",4) == 0){
				sscanf(var, "eq1=%hhd", &opciones.eq1);
				sendToQueue(WEB_BANDS, 0x0000 | opciones.eq1);
			}
			else if (strncmp(var, "eq2=",4) == 0){
				sscanf(var, "eq2=%hhd", &opciones.eq2);
				sendToQueue(WEB_BANDS, 0x0100 | opciones.eq2);
			}
			else if (strncmp(var, "eq3=",4) == 0){
				sscanf(var, "eq3=%hhd", &opciones.eq3);	
				sendToQueue(WEB_BANDS, 0x0200 | opciones.eq3);				
			}
			else if (strncmp(var, "eq4=",4) == 0){
				sscanf(var, "eq4=%hhd", &opciones.eq4);
				sendToQueue(WEB_BANDS, 0x0300 | opciones.eq4);
			}
			else if (strncmp(var, "eq5=",4) == 0){
				sscanf(var, "eq5=%hhd", &opciones.eq5);
				sendToQueue(WEB_BANDS, 0x0400 | opciones.eq5);
			}
			else if (strncmp(var, "canciones=",10) == 0){
				searchSong(&var[10]);
			}
			else if (strncmp(var, "prev_song.x=",12) == 0){
				sendToQueue(WEB_PREV_SONG, 0);
			}
			else if (strncmp(var,"play_song.x=", 12) == 0){
				sendToQueue(WEB_PLAY_PAUSE, 0);
			}
			else if (strncmp(var, "next_song.x=",11) == 0){
				sendToQueue(WEB_NEXT_SONG, 0);
			}
			else if (strncmp(var, "loop_song.x=",11) == 0){
				sendToQueue(WEB_LOOP, 0);
			}
		}
  } while (data);


}

// Generate dynamic web data from a script line.
uint32_t netCGI_Script (const char *env, char *buf, uint32_t buflen, uint32_t *pcgi) {
//  int32_t socket;
//  netTCP_State state;
//  NET_ADDR r_client;
//  const char *lang;
  uint32_t len = 0U;
//  uint8_t id;
//  static uint32_t adv;
//  netIF_Option opt = netIF_OptionMAC_Address;
//  int16_t      typ = 0;
	
	switch(env[0]){
		case 'i':
			// Cases for index
			switch (env[2]){
				case '1':
				// Case for Radio Input
					len = sprintf (buf, &env[4], opciones.entrada == RADIO ? "checked" :"");
				break;
				case '2':
				// Case for MP3 Input	
					len = sprintf (buf, &env[4], opciones.entrada == MP3 ? "checked" :"");
				break;
				case '3':
				// Case for Altavoz Output
					len = sprintf (buf, &env[4], opciones.salida == ALTAVOZ ? "checked" :"");
				break;
				case '4':
				// Case for Auriculares Output
					len = sprintf (buf, &env[4], opciones.salida == AURICULARES ? "checked" :"");
				break;
			}
		break;
			
		case 'm':
			// Cases for mp3
			switch (env[2]){
				case '6':
				// Case for volumen	
					len = sprintf (buf, &env[4], opciones.vol);
				break;
				case '7':
				// Case for Altavoz Output
					len = sprintf (buf, &env[4], opciones.salida == ALTAVOZ ? "checked" :"");
				break;
				case '8':
				// Case for Auriculares Output
					len = sprintf (buf, &env[4], opciones.salida == AURICULARES ? "checked" :"");
				break;
				default:
				// Case for songs	
					if (env[2] - 'a' < song_cnt) {
						len = sprintf (buf, &env[4], canciones[env[2] - 'a']);
					} else {
						len = 0;
					}
					break;
			}
		break;
			
		case 'r':
			// Cases for Radio
			switch (env[2]){
				case '1':
				// Case for frecuency
					len = sprintf (buf, &env[4], opciones.freq_actual);
				break;
				case '2':
				// Case for volumen
					len = sprintf (buf, &env[4], opciones.vol);
				break;
				case '3':
				// Case for Altavoz Output
					len = sprintf (buf, &env[4], opciones.salida == ALTAVOZ ? "checked" :"");
				break;
				case '4':
				// Case for Auriculares Output
					len = sprintf (buf, &env[4], opciones.salida == AURICULARES ? "checked" :"");
				break;
			}
		break;
			
		case 'f':
			// Cases for filtros
			switch (env[2]){
				case '1':
				// Case for eq1
					len = sprintf (buf, &env[4], opciones.eq1);
				break;
				case '2':
				// Case for eq2
					len = sprintf (buf, &env[4], opciones.eq2);
				break;
				case '3':
				// Case for eq3
					len = sprintf (buf, &env[4], opciones.eq3);
				break;
				case '4':
				// Case for eq4
					len = sprintf (buf, &env[4], opciones.eq4);
				break;
				case '5':
				// Case for eq5	
					len = sprintf (buf, &env[4], opciones.eq5);
				break;
				case '6':
				// Case for volumen	
					len = sprintf (buf, &env[4], opciones.vol);
				break;
				case '7':
				// Case for Altavoz Output
					len = sprintf (buf, &env[4], opciones.salida == ALTAVOZ ? "checked" :"");
				break;
				case '8':
				// Case for Auriculares Output
					len = sprintf (buf, &env[4], opciones.salida == AURICULARES ? "checked" :"");
				break;
			}
		break;
			
		case 'h':
			len = sprintf (buf, &env[1], opciones.dia , opciones.mes, opciones.ano, opciones.horas, opciones.min, opciones.seg);
		break;

		
	}

	return (len);
}

void sendToQueue(web_msg_type_t type, uint16_t payload){
	msgToMain.web_msg.type = type;
	msgToMain.web_msg.payload = payload;
	//osMessageQueuePut(ctrl_in_queue, &msgToMain, NULL, 0);
}

void searchSong(char* name){
	for (int i = 0; i < 25; i++){
		if (strcmp(name, canciones[i]) == 0){
			sendToQueue(WEB_SONG, i);
			return;
		}
	}
}

#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic pop
#endif
