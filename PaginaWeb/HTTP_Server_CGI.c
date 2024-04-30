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
#include <stdlib.h>
#include "Board_Buttons.h"

#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic push
#pragma  clang diagnostic ignored "-Wformat-nonliteral"
#endif



//// Local variables.
static uint8_t ip_addr[NET_ADDR_IP6_LEN];
static char    ip_string[40];

// My structure of CGI status variable.
typedef struct {
  uint8_t idx;
  uint8_t unused[3];
} MY_BUF;
#define MYBUF(p)        ((MY_BUF *)p)

typedef enum {
	RADIO			= 0,
	MP3			 	= 1
} input_t;

typedef enum {
	ALTAVOZ 			= 0,
	AURICULARES 	= 1
} output_t;

typedef enum {
	SEEKUP   			= 0,
	SEEKDOWN			= 1
} seek_t;

typedef struct {
	input_t entrada;
	output_t salida;
	uint8_t bajo_consumo;
	uint8_t vol, prev_vol;
	uint8_t mute;
	float consumo;
	uint32_t freq_actual;
	uint8_t cancion_sel;
	int8_t eq1, eq2, eq3, eq4, eq5;
	uint8_t horas, min, seg;
	uint8_t dia, mes, ano;
	seek_t seek;
} options;

char canciones [5][10] = {
	"Cancion 1",
	"Cancion 2",
	"Cancion 3",
	"Cancion 4",
	"Cancion 5"
};


options opciones = {
	.dia = 29,
	.mes = 04,
	.ano = 24,
	.horas = 18,
	.min = 51,
	.seg = 33
};

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
	if (Buttons_GetState() == 1){
		opciones.dia += 1;
		opciones.ano -= 1;
	}

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
				} 
				else if (strncmp(var, "entrada=mp3",11) == 0){
					opciones.entrada = MP3;
				}
			}
			else if (strncmp(var, "salida=",7) == 0){
				if (strncmp(var, "salida=altavoz",14) == 0){
					opciones.salida = ALTAVOZ;
				} 
				else if (strncmp(var, "salida=cascos",13) == 0){
					opciones.salida = AURICULARES;
				}
			}
			else if (strncmp(var, "consumo=",8) == 0){
				opciones.bajo_consumo = 1;
			}
			else if (strncmp(var, "frec_sint=",10) == 0){
				sscanf(var, "frec_sint=%d", &opciones.freq_actual);
			}
			else if (strncmp(var, "seekup=",7) == 0){
				opciones.seek = SEEKUP;
			}
			else if (strncmp(var, "seekdown=",9) == 0){
				opciones.seek = SEEKDOWN;
			}
			else if (strncmp(var, "mute=",5) == 0){
				opciones.mute =! opciones.mute;
				opciones.vol = (opciones.mute == 1 ? 0 : opciones.prev_vol);
			}
			else if (strncmp(var, "formatted_vol=",14) == 0){
				sscanf(var, "formatted_vol=%hhd", &opciones.vol);
				opciones.prev_vol = opciones.vol;
			}
			else if (strncmp(var, "save_conf.x=",12) == 0){
				//Guardar configuración
			}
			else if (strncmp(var, "eq1=",4) == 0){
				sscanf(var, "eq1=%hhd", &opciones.eq1);
			}
			else if (strncmp(var, "eq2=",4) == 0){
				sscanf(var, "eq2=%hhd", &opciones.eq2);
			}
			else if (strncmp(var, "eq3=",4) == 0){
				sscanf(var, "eq3=%hhd", &opciones.eq3);	
			}
			else if (strncmp(var, "eq4=",4) == 0){
				sscanf(var, "eq4=%hhd", &opciones.eq4);
			}
			else if (strncmp(var, "eq5=",4) == 0){
				sscanf(var, "eq5=%hhd", &opciones.eq5);
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
				case '1':
				// Case for 1º Song	
					len = sprintf (buf, &env[4], canciones[0]);
				break;
				case '2':
				// Case for 2º Song	
					len = sprintf (buf, &env[4], canciones[1]);
				break;
				case '3':
				// Case for 3º Song	
					len = sprintf (buf, &env[4], canciones[2]);
				break;
				case '4':
				// Case for 4º Song	
					len = sprintf (buf, &env[4], canciones[3]);
				break;
				case '5':
				// Case for 5º Song	
					len = sprintf (buf, &env[4], canciones[4]);
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

#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic pop
#endif
