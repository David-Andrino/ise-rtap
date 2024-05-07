#include "gui.h"
#include "stdio.h"
#include "math.h"

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN && LV_MEM_SIZE < (64ul * 1024ul)
    #error Insufficient memory. Please set LV_MEM_SIZE to at least 64KB (64ul * 1024ul).  128KB is recommended.
#endif

#define TITLE "Real Time Audio Processor"
#define ABOUT "Ingenieria de Sistemas Electronicos 2024"
#define DRAGGABBLE 0
#define MAX_VOLUME 10
/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
	DISP_SMALL,
	DISP_MEDIUM,
	DISP_LARGE,
} disp_size_t;

static gui_data_t data = {
	.vol = 10,
	.num_canciones = 10,
	.songs[0] = "Cancion 0",
	.songs[1] = "Cancion 1",
	.songs[2] = "Cancion 2",
	.songs[3] = "Cancion 3",
	.songs[4] = "Cancion 4",
	.songs[5] = "Cancion 5",
	.songs[6] = "Cancion 6",
	.songs[7] = "Cancion 7",
	.songs[8] = "Cancion 8",
	.songs[9] = "Cancion 197 mil millones",
	.fcentral[0] = "20Hz",
	.fcentral[1] = "110Hz",
	.fcentral[2] = "630Hz",
	.fcentral[3] = "3.5kHz",
	.fcentral[4] = "15kHz",
};

lv_obj_t * scale_consumo; 	// Tiene que ser global para modificarlo desde async_cb
static lv_obj_t *needle_consumo, *needle_freq;
lv_obj_t * label_consumo;

extern osMessageQueueId_t ctrl_in_queue;
static msg_ctrl_t msg_to_main = {
	.type = MSG_LCD
};
/**********************
 *  STATIC PROTOTYPES
 **********************/
static void home_create(lv_obj_t * parent);
static void create_radio_content(lv_obj_t * tabview);
static void create_mp3_content(lv_obj_t * tabview);
static void create_filters_content(lv_obj_t * tabview);

static void crear_panel_titulo       (lv_obj_t * container);
static void crear_panel_consumo      (lv_obj_t * container);
static void crear_panel_freq         (lv_obj_t * container);
static void crear_panel_mp3          (lv_obj_t * container);
static void crear_panel_snaps_mp3    (lv_obj_t * container);
static void crear_panel_control_mp3  (lv_obj_t * container);
static void crear_panel_canciones    (lv_obj_t * container);
static void crear_panel_seek         (lv_obj_t * container);
static void crear_panel_salida       (lv_obj_t * container, lv_obj_t * btn_headphones, lv_obj_t * btn_speakers);
static void crear_panel_camino_audio (lv_obj_t * container);
static void crear_panel_config_rapida(lv_obj_t * container);
static void crear_panel_volumen      (lv_obj_t * container, lv_obj_t * vol_slider);

static void ta_event_cb(lv_event_t * e);
static void slider_event_cb(lv_event_t * e);
static void tabview_delete_event_cb(lv_event_t * e);

static void drag_event_handler(lv_event_t * e);
static void frequency_set_cb(lv_event_t * e);
static void seek_up_cb(lv_event_t * e);
static void seek_down_cb(lv_event_t * e);
static void guardar_cadena_cb(lv_event_t * e);
static void guardar_cadena_cb(lv_event_t * e);
static void set_channel_from_list_cb(lv_event_t * e);
static void volume_cb(lv_event_t * e);
static void headphones_cb(lv_event_t * e);
static void speakers_cb(lv_event_t * e);
static void mute_cb(lv_event_t * e);
static void save_cb(lv_event_t * e){}
static void mp3_cb(lv_event_t * e);       // Seleccion del mp3 como entrada de audio
static void mp3_ctrl_cb(lv_event_t * e);  // Botones play/pause, next song, prev song
static void mp3_list_cb(lv_event_t * e);  // Seleccion de cancion desde la lista
static void radio_cb(lv_event_t * e);
static void low_power_cb(lv_event_t * e){ EnterStandbyMode(); }
static void filter_cb(lv_event_t * e);
	
static void cambiar_estilo_entrada(int in);
static void cambiar_estilo_salida(int out);
static void set_freq(lv_obj_t * obj, uint16_t val);
static void set_vol(lv_obj_t * slider, uint8_t vol);
static void set_filter(uint8_t band, int8_t value);
/**********************
 *  STATIC VARIABLES
 **********************/
static disp_size_t disp_size;

static lv_obj_t * tv;
static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_subtitle;
static lv_style_t style_icon;
static lv_style_t style_bullet;
static lv_style_t style_ctrl_mp3;

static lv_style_t btn_style_base;     /* Cuando un boton no está seleccionado: azul */
static lv_style_t btn_style_sel;      /* Cuando un boton está seleccionado: blanco  */
	
static const lv_font_t * font_ultralarge;
static const lv_font_t * font_large;
static const lv_font_t * font_medium;
static const lv_font_t * font_normal;

//static lv_obj_t * label_volumen;
static lv_obj_t *volume_slider_radio, *volume_slider_mp3, *volume_slider_filtros;		// Necesario que sea global para poder copiar el valor. No puede haber solo un objeto compartido porque falla (Necesario uno por tab).

static lv_obj_t * panel_ctrl_mp3;

/* Configuración rápida */
static lv_obj_t * btn_headphones_conf_rapida;
static lv_obj_t * btn_speakers_conf_rapida;
static lv_obj_t * btn_mp3_conf_rapida;
static lv_obj_t * btn_radio_conf_rapida;

/* Radio */
static lv_obj_t * btn_headphones_radio;
static lv_obj_t * btn_speakers_radio;
static lv_obj_t * textarea_freq;
static lv_obj_t * slider_freq;
static lv_obj_t * label_cadena, *label_cadena_txt; 
static lv_obj_t * list_cadenas;
static float lista_freq_guardadas[15];
static int index = 0;

/* MP3 */
static lv_obj_t * label_title_mp3, *label_cancion;
static lv_obj_t * btn_headphones_mp3;
static lv_obj_t * btn_speakers_mp3;
static lv_obj_t * btn_play_mp3, *btn_prev_mp3, *btn_next_mp3;

/* Filtros */
static lv_obj_t * btn_headphones_filtros;
static lv_obj_t * btn_speakers_filtros;
static lv_obj_t * btn_mp3_filtros;
static lv_obj_t * btn_radio_filtros;
static lv_obj_t * slider_b1, * slider_b2, * slider_b3, * slider_b4, * slider_b5;

void async_cb(void * new_data){
    lcd_out_msg_t * msg = new_data;
		switch(msg->type){
			case LCD_OUT_VOL: // No cambia el volumen: Solo mutea
				set_vol(NULL, msg->payload);
			break;
			case LCD_OUT_BANDS: 
				set_filter(msg->payload >> 8, msg->payload & 0x00FF);
			break;
			case LCD_OUT_RADIO_FREQ:
				set_freq(NULL, msg->payload);
			break;
//			case LCD_OUT_SONG:
//				
//			break;
			case LCD_OUT_INPUT_SEL: // No hace nada
				cambiar_estilo_entrada(msg->payload);
			break;
			case LCD_OUT_OUTPUT_SEL:
				cambiar_estilo_salida(msg->payload);
			break;
			case LCD_OUT_CONS:
				;
				char str[32]; /* Buffer to hold the string */
				
				sprintf(str, "Consumo actual %d mA", msg->payload);

				lv_label_set_text(label_consumo, str);
				lv_scale_set_line_needle_value(scale_consumo, needle_consumo, 80, msg->payload);
			break;
//			case LCD_SAVE_SD:
//				
//			break;
//			case LCD_LOW_POWER:
//				
//			break;
//			case LCD_LOOP:
//				
//			break;
//			case LCD_SEEK:
//				
//			break;
//			case LCD_NEXT_SONG:
//				
//			break;
//			case LCD_PREV_SONG:
//				
//			break;
//			case LCD_PLAY_PAUSE:
//				
//			break;
			default:
			break;
		}
}

void lv_gui(){
	disp_size = DISP_LARGE;
	font_ultralarge = LV_FONT_DEFAULT;
	font_large = LV_FONT_DEFAULT;
	font_medium = LV_FONT_DEFAULT;
	font_normal = LV_FONT_DEFAULT;
	int32_t tab_h;
	tab_h = 70;
	
#if LV_FONT_MONTSERRAT_36
		font_ultralarge     = &lv_font_montserrat_36;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_36 is not enabled. Using LV_FONT_DEFAULT instead.");
#endif
#if LV_FONT_MONTSERRAT_24
		font_large     = &lv_font_montserrat_24;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_24 is not enabled. Using LV_FONT_DEFAULT instead.");
#endif
#if LV_FONT_MONTSERRAT_18
		font_medium     = &lv_font_montserrat_18;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_18 is not enabled. Using LV_FONT_DEFAULT instead.");
#endif
#if LV_FONT_MONTSERRAT_16
		font_normal    = &lv_font_montserrat_16;
#else
		LV_LOG_WARN("LV_FONT_MONTSERRAT_16 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
    
#if LV_USE_THEME_DEFAULT
    lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK,
                          font_normal);
#endif

	lv_style_init(&style_text_muted);
	lv_style_set_text_opa(&style_text_muted, LV_OPA_50);

	lv_style_init(&style_title);
	lv_style_set_text_font(&style_title, font_large);
	
	lv_style_init(&style_subtitle);
	lv_style_set_text_font(&style_subtitle, font_medium);
	
	lv_style_init(&style_icon);
	lv_style_set_text_color(&style_icon, lv_theme_get_color_primary(NULL));
	lv_style_set_text_font(&style_icon, font_large);

	lv_style_init(&style_bullet);
	lv_style_set_border_width(&style_bullet, 0);
	lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);

	tv = lv_tabview_create(lv_screen_active());
	lv_tabview_set_tab_bar_size(tv, tab_h);
	lv_obj_add_event_cb(tv, tabview_delete_event_cb, LV_EVENT_DELETE, NULL);

	lv_obj_set_style_text_font(lv_screen_active(), font_normal, 0);
	
	lv_style_init(&btn_style_base);
	lv_style_set_bg_color(&btn_style_base, lv_palette_main(LV_PALETTE_BLUE));
	lv_style_set_border_color(&btn_style_base, lv_palette_darken(LV_PALETTE_LIGHT_BLUE, 3));
	lv_style_set_text_color(&btn_style_base, lv_color_white());
	
	lv_style_init(&btn_style_sel);
	lv_style_set_bg_color(&btn_style_sel, lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 3));
	lv_style_set_border_color(&btn_style_sel, lv_palette_darken(LV_PALETTE_BLUE, 3));
	lv_style_set_text_color(&btn_style_sel, lv_color_black());
	
	lv_obj_t * t1 = lv_tabview_add_tab(tv, "Home");
	lv_obj_t * t2 = lv_tabview_add_tab(tv, "Radio");
	lv_obj_t * t3 = lv_tabview_add_tab(tv, "MP3");
	lv_obj_t * t4 = lv_tabview_add_tab(tv, "Filtros");
	
	lv_obj_t * tab_bar = lv_tabview_get_tab_bar(tv);
	lv_obj_set_style_pad_left(tab_bar, LV_HOR_RES / 2, 0);
	lv_obj_t * logo = lv_image_create(tab_bar);
	lv_obj_add_flag(logo, LV_OBJ_FLAG_IGNORE_LAYOUT);
	/* Sin espacio! */
//        LV_IMAGE_DECLARE(logo_rtap);
//        lv_image_set_src(logo, &logo_rtap);
	lv_obj_align(logo, LV_ALIGN_LEFT_MID, -LV_HOR_RES / 2 + 25, 0);

	lv_obj_t * label = lv_label_create(tab_bar);
	lv_obj_add_style(label, &style_title, 0);
	lv_obj_add_flag(label, LV_OBJ_FLAG_IGNORE_LAYOUT);
	lv_label_set_text_fmt(label, "RTAP");
	lv_obj_align_to(label, logo, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);
	
	home_create(t1);
	create_radio_content(t2);
	create_mp3_content(t3);
	create_filters_content(t4);
//  color_changer_create(tv);
}


static void home_create(lv_obj_t * tabview){
	/* Titulo */
	lv_obj_t * panel_title = lv_obj_create(tabview);
	crear_panel_titulo(panel_title);
	
	/* Configuracion rapida */
	lv_obj_t * panel_salida = lv_obj_create(tabview);
	lv_obj_set_height(panel_salida, LV_SIZE_CONTENT);
	lv_obj_set_width(panel_salida, LV_SIZE_CONTENT);
	crear_panel_config_rapida(panel_salida);
 
	/*Create the third panel*/
	lv_obj_t * panel_consumo = lv_obj_create(tabview);
  crear_panel_consumo(panel_consumo);
	lv_obj_set_height(panel_consumo, lv_pct(100));

	static int32_t grid_main_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_main_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

	lv_obj_set_grid_dsc_array(tabview, grid_main_col_dsc, grid_main_row_dsc);
	
	/* Colocacion paneles*/
	lv_obj_set_grid_cell(panel_salida,  LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(panel_consumo, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(panel_title,   LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_STRETCH, 1, 1);
}

static void create_radio_content(lv_obj_t * tabview) {
	lv_obj_t * panel_titulo = lv_obj_create(tabview);
	crear_panel_titulo(panel_titulo);
	
	lv_obj_t * panel_freq = lv_obj_create(tabview);
	lv_obj_set_height(panel_freq, LV_SIZE_CONTENT);
	lv_obj_set_width(panel_freq, LV_SIZE_CONTENT);
	crear_panel_freq(panel_freq);												// Aqui se crea todo el panel de la frecuencia
	
	/* Volumen */
	lv_obj_t * panel_vol = lv_obj_create(tabview);
	lv_obj_set_height(panel_vol, LV_SIZE_CONTENT);
	lv_obj_set_width(panel_vol , LV_SIZE_CONTENT);
	lv_obj_t * label_volumen;
	label_volumen = lv_label_create(panel_vol);
	lv_label_set_text_fmt(label_volumen, "Volumen");
	lv_obj_add_style(label_volumen, &style_subtitle, 0);  
	
	volume_slider_radio = lv_slider_create(panel_vol);
	lv_obj_add_event_cb(volume_slider_radio, volume_cb, LV_EVENT_RELEASED, NULL);
	lv_slider_set_value(volume_slider_radio, data.vol, LV_ANIM_ON);	
	lv_slider_set_range(volume_slider_radio, 0, MAX_VOLUME);
	
	lv_obj_t * mute_btn = lv_btn_create(panel_vol);
	lv_obj_add_event_cb(mute_btn, mute_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_mute_btn = lv_label_create(mute_btn);
	lv_label_set_text_fmt(label_mute_btn, "%s  Mute", LV_SYMBOL_MUTE);
	
	static int32_t grid_vol_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_vol_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Slider    */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Mute      */
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(panel_vol, grid_vol_col_dsc, grid_vol_row_dsc);
	
	lv_obj_set_grid_cell(label_volumen,       LV_GRID_ALIGN_CENTER,  0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(volume_slider_radio, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
	lv_obj_set_grid_cell(mute_btn,            LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 4, 1);
	
	static lv_obj_t * panel_salida;
	panel_salida = lv_obj_create(tabview);
	lv_obj_set_height(panel_salida, LV_SIZE_CONTENT);
	lv_obj_set_width(panel_salida , LV_SIZE_CONTENT);
	btn_headphones_radio = lv_btn_create(panel_salida);
	btn_speakers_radio = lv_btn_create(panel_salida);
	crear_panel_salida(panel_salida, btn_headphones_radio, btn_speakers_radio);	
	
	/* Define the grid */
	static int32_t grid_col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_row_dsc[] = {
		LV_GRID_CONTENT,          
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(tabview, grid_col_dsc, grid_row_dsc);
	
	lv_obj_set_grid_cell(panel_freq,   LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
//	lv_obj_set_grid_cell(panel_seek,   LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(panel_vol,    LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(panel_salida, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
	lv_obj_set_grid_cell(panel_titulo, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_STRETCH, 2, 1);
}
static void create_mp3_content(lv_obj_t * tabview){
	lv_obj_t * panel_titulo = lv_obj_create(tabview);
	crear_panel_titulo(panel_titulo);
	
	lv_obj_t * panel_mp3 = lv_obj_create(tabview);
	lv_obj_set_height(panel_mp3, LV_SIZE_CONTENT);
	lv_obj_set_width(panel_mp3, LV_SIZE_CONTENT);
	crear_panel_mp3(panel_mp3);

	/* Volumen */
	lv_obj_t * panel_vol = lv_obj_create(tabview);
	lv_obj_set_height(panel_vol, LV_SIZE_CONTENT);
	lv_obj_set_width(panel_vol , LV_SIZE_CONTENT);
	lv_obj_t * label_volumen;
	label_volumen = lv_label_create(panel_vol);
	lv_label_set_text_fmt(label_volumen, "Volumen");
	lv_obj_add_style(label_volumen, &style_subtitle, 0);  
	
	volume_slider_mp3 = lv_slider_create(panel_vol);
	lv_obj_add_event_cb(volume_slider_mp3, volume_cb, LV_EVENT_RELEASED, NULL);
	lv_slider_set_value(volume_slider_mp3, data.vol, LV_ANIM_ON);	
	lv_slider_set_range(volume_slider_mp3, 0, MAX_VOLUME);
	
	lv_obj_t * mute_btn = lv_btn_create(panel_vol);
	lv_obj_add_event_cb(mute_btn, mute_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_mute_btn = lv_label_create(mute_btn);
	lv_label_set_text_fmt(label_mute_btn, "%s  Mute", LV_SYMBOL_MUTE);
	
	static int32_t grid_vol_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_vol_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Slider    */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Mute      */
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(panel_vol, grid_vol_col_dsc, grid_vol_row_dsc);
	
	lv_obj_set_grid_cell(label_volumen,     LV_GRID_ALIGN_CENTER,  0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(volume_slider_mp3, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
	lv_obj_set_grid_cell(mute_btn,          LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 4, 1);

	static lv_obj_t * panel_salida; 
	panel_salida = lv_obj_create(tabview);
	lv_obj_set_height(panel_salida, LV_SIZE_CONTENT);
	lv_obj_set_width(panel_salida , LV_SIZE_CONTENT);
	btn_headphones_mp3 = lv_btn_create(panel_salida);
	btn_speakers_mp3 = lv_btn_create(panel_salida);
	crear_panel_salida(panel_salida, btn_headphones_mp3, btn_speakers_mp3);
	
	/* Define the grid */
	static int32_t grid_col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_row_dsc[] = {
		LV_GRID_CONTENT,          
		LV_GRID_CONTENT,
		LV_GRID_CONTENT,
		LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(tabview, grid_col_dsc, grid_row_dsc);
	
	lv_obj_set_grid_cell(panel_mp3,   LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
	lv_obj_set_grid_cell(panel_vol,    LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(panel_salida, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
	lv_obj_set_grid_cell(panel_titulo, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_STRETCH, 2, 1);
}


static void create_filters_content(lv_obj_t * tabview){
	lv_obj_t * panel_title = lv_obj_create(tabview);	  // En este panel va el titulo y los nombres
	
	lv_obj_t * panel1 = lv_obj_create(tabview);			    // En este panel van los filtros del ecualizador
	lv_obj_set_height(panel1, lv_pct(95));
	lv_obj_set_width(panel1, LV_SIZE_CONTENT);
	
	lv_obj_t * panel_config = lv_obj_create(tabview);		// En este panel va lo de guardar config
	lv_obj_set_height(panel_config, lv_pct(43));
	lv_obj_set_width(panel_config, LV_SIZE_CONTENT);
	
	lv_obj_t * panel_vol = lv_obj_create(tabview);			     // Aqui el volumen
	lv_obj_set_height(panel_vol, lv_pct(52));
	lv_obj_set_width(panel_vol, LV_SIZE_CONTENT);
	
	lv_obj_t * panel_salida = lv_obj_create(tabview);			// Y aqui la salida
	lv_obj_set_height(panel_salida, lv_pct(95));
	lv_obj_set_width(panel_salida, LV_SIZE_CONTENT);
	
	/* Titulo */
	crear_panel_titulo(panel_title);
	
//	lv_obj_t * label_patata = lv_label_create(panel_title);
//	lv_label_set_text(label_patata, "Patata");
	
	/* Ecualizador */
	lv_obj_t * ecualizador_label = lv_label_create(panel1);
	lv_label_set_text(ecualizador_label, "Ecualizador");
	lv_obj_add_style(ecualizador_label, &style_subtitle, 0);
	
	#define TAM_V 220
	/* Banda 1 */
	slider_b1 = lv_slider_create(panel1);
	lv_obj_set_size(slider_b1, 20, TAM_V);
	lv_obj_add_event_cb(slider_b1, filter_cb, LV_EVENT_RELEASED, NULL);
	lv_slider_set_range(slider_b1, -2, 2);
	lv_slider_set_value(slider_b1, 0, LV_ANIM_ON);
	
	lv_obj_t * slider_b1_label = lv_label_create(panel1);
	lv_label_set_text(slider_b1_label, data.fcentral[0]);
	lv_obj_add_style(slider_b1_label, &style_text_muted, 0);
	
	/* Banda 2 */
	slider_b2 = lv_slider_create(panel1);
	lv_obj_set_size(slider_b2, 20, TAM_V);
	lv_obj_add_event_cb(slider_b2, filter_cb, LV_EVENT_RELEASED, NULL);
	lv_slider_set_value(slider_b2, 0, LV_ANIM_OFF);
	lv_slider_set_range(slider_b2, -9, 9);
	
	lv_obj_t * slider_b2_label = lv_label_create(panel1);
	lv_label_set_text(slider_b2_label, data.fcentral[1]);
	lv_obj_add_style(slider_b2_label, &style_text_muted, 0);
	
	/* Banda 3 */
	slider_b3 = lv_slider_create(panel1);
	lv_obj_set_size(slider_b3, 20, TAM_V);
	lv_obj_add_event_cb(slider_b3, filter_cb, LV_EVENT_RELEASED, NULL);
	lv_slider_set_value(slider_b3, 0, LV_ANIM_OFF);
	lv_slider_set_range(slider_b3, -9, 9);
	
	lv_obj_t * slider_b3_label = lv_label_create(panel1);
	lv_label_set_text(slider_b3_label, data.fcentral[2]);
	lv_obj_add_style(slider_b3_label, &style_text_muted, 0);
	
	/* Banda 4 */
	slider_b4 = lv_slider_create(panel1);
	lv_obj_set_size(slider_b4, 20, TAM_V);
	lv_obj_add_event_cb(slider_b4, filter_cb, LV_EVENT_RELEASED, NULL);
	lv_slider_set_value(slider_b4, 0, LV_ANIM_OFF);
	lv_slider_set_range(slider_b4, -9, 9);
	
	lv_obj_t * slider_b4_label = lv_label_create(panel1);
	lv_label_set_text(slider_b4_label, data.fcentral[3]);
	lv_obj_add_style(slider_b4_label, &style_text_muted, 0);
	
	/* Banda 5 */
	slider_b5 = lv_slider_create(panel1);
	lv_obj_set_size(slider_b5, 20, TAM_V);
	lv_obj_add_event_cb(slider_b5, filter_cb, LV_EVENT_RELEASED, NULL);
	lv_slider_set_value(slider_b5, 0, LV_ANIM_OFF);
	lv_slider_set_range(slider_b5, -9, 9);
	
	lv_obj_t * slider_b5_label = lv_label_create(panel1);
	lv_label_set_text(slider_b5_label, data.fcentral[4]);
	lv_obj_add_style(slider_b5_label, &style_text_muted, 0);
	
	/* Volumen */
//	lv_obj_set_height(panel_vol, LV_SIZE_CONTENT);
//	lv_obj_set_width(panel_vol , LV_SIZE_CONTENT);
	lv_obj_t * label_volumen;
	label_volumen = lv_label_create(panel_vol);
	lv_label_set_text_fmt(label_volumen, "Volumen");
	lv_obj_add_style(label_volumen, &style_subtitle, 0);  
	
	volume_slider_filtros = lv_slider_create(panel_vol);
	lv_obj_add_event_cb(volume_slider_filtros, volume_cb, LV_EVENT_RELEASED, NULL);
	lv_slider_set_value(volume_slider_filtros, data.vol, LV_ANIM_ON);	
	lv_slider_set_range(volume_slider_filtros, 0, MAX_VOLUME);
	
	lv_obj_t * mute_btn = lv_btn_create(panel_vol);
	lv_obj_add_event_cb(mute_btn, mute_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_mute_btn = lv_label_create(mute_btn);
	lv_label_set_text_fmt(label_mute_btn, "%s  Mute", LV_SYMBOL_MUTE);
	
	#if DRAGGABBLE
	lv_obj_add_event_cb(panel_title, drag_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(panel1, drag_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(panel_config, drag_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(panel_vol, drag_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(panel_salida, drag_event_handler, LV_EVENT_ALL, NULL);
	#endif
	
	static int32_t grid_vol_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_vol_row_dsc[] = {
			LV_GRID_FR(1),  /* Subtitulo */
//			5,                /* Separador */
			LV_GRID_FR(2),  /* Slider    */
//			5,                /* Separador */
			LV_GRID_FR(2),  /* Mute      */
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(panel_vol, grid_vol_col_dsc, grid_vol_row_dsc);
	
	lv_obj_set_grid_cell(label_volumen,         LV_GRID_ALIGN_CENTER,  0, 1, LV_GRID_ALIGN_START,  0, 1);
	lv_obj_set_grid_cell(volume_slider_filtros, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
	lv_obj_set_grid_cell(mute_btn,              LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
	
	
	/* Guardar configuracion */
	lv_obj_t * label_config = lv_label_create(panel_config);
	lv_label_set_text_fmt(label_config, "Guardar config");
	
//	lv_obj_t * save_btn = lv_list_add_btn(list_save, LV_SYMBOL_SAVE, "Save");//lv_btn_create(panel_config);
	lv_obj_t * save_btn = lv_btn_create(panel_config);
	lv_obj_add_event_cb(save_btn, save_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_save_config_btn = lv_label_create(save_btn);
	lv_label_set_text_fmt(label_save_config_btn, "%s  Save", LV_SYMBOL_SAVE);
	
	/* Botones salida de audio */
	crear_panel_camino_audio(panel_salida);	
	
	/* Para la tab */
	static int32_t grid_main_col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_main_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	
	/* Para el ecualizador */
	static int32_t grid_2_col_dsc[] = {LV_GRID_FR(1), 10, LV_GRID_FR(1), 10, LV_GRID_FR(1), 10, LV_GRID_FR(1), 10, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_2_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Sliders   */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Bandas */
			LV_GRID_TEMPLATE_LAST
	};
	
	/* Para guardar configuracion */
	static int32_t grid_conf_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_conf_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
//			5,                /* Separador */
			LV_GRID_FR(1),  /* Boton     */
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(tabview, grid_main_col_dsc, grid_main_row_dsc);
	
	/* Titulo: Real Time Audio Processor & names. En el fondo de la pagina */
	lv_obj_set_grid_cell(panel_title, LV_GRID_ALIGN_STRETCH, 0, 3, LV_GRID_ALIGN_STRETCH, 2, 1);
//	lv_obj_set_grid_cell(label_patata, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
	
	/* Ecualizador */
	lv_obj_set_grid_cell(panel1, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
	lv_obj_set_grid_dsc_array(panel1, grid_2_col_dsc, grid_2_row_dsc);
	
	lv_obj_set_grid_cell(ecualizador_label, LV_GRID_ALIGN_CENTER, 0, 9, LV_GRID_ALIGN_STRETCH, 0, 1);
	
	lv_obj_set_grid_cell(slider_b1, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
	lv_obj_set_grid_cell(slider_b2, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
	lv_obj_set_grid_cell(slider_b3, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
	lv_obj_set_grid_cell(slider_b4, LV_GRID_ALIGN_CENTER, 6, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
	lv_obj_set_grid_cell(slider_b5, LV_GRID_ALIGN_CENTER, 8, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
	
	lv_obj_set_grid_cell(slider_b1_label, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_STRETCH, 4, 1);
	lv_obj_set_grid_cell(slider_b2_label, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_STRETCH, 4, 1);
	lv_obj_set_grid_cell(slider_b3_label, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_STRETCH, 4, 1);
	lv_obj_set_grid_cell(slider_b4_label, LV_GRID_ALIGN_CENTER, 6, 1, LV_GRID_ALIGN_STRETCH, 4, 1);
	lv_obj_set_grid_cell(slider_b5_label, LV_GRID_ALIGN_CENTER, 8, 1, LV_GRID_ALIGN_STRETCH, 4, 1);
	
	/* Volumen */
	lv_obj_set_grid_cell(panel_vol, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	
	/* Guardar config */
	lv_obj_set_grid_cell(panel_config, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
	lv_obj_set_grid_dsc_array(panel_config, grid_conf_col_dsc, grid_conf_row_dsc);
	
	lv_obj_set_grid_cell(label_config, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(save_btn, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
	
	/* Salida */
	lv_obj_set_grid_cell(panel_salida, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 2);

}
static void drag_event_handler(lv_event_t * e){
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * obj = lv_event_get_target(e);
	lv_obj_t * parent = lv_obj_get_parent(obj);
	lv_obj_t * screen = lv_obj_get_parent(lv_obj_get_parent(obj));
	
	lv_indev_t * indev = lv_indev_get_act();
	if(indev == NULL)  return;
	switch(code) {
    case LV_EVENT_PRESSED:
      lv_obj_remove_flag(lv_tabview_get_content(screen), LV_OBJ_FLAG_SCROLLABLE);
			lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
			
			lv_obj_set_layout(parent, LV_LAYOUT_NONE);
			break;
    case LV_EVENT_LONG_PRESSED:
			break;
    case LV_EVENT_LONG_PRESSED_REPEAT:
			lv_obj_set_layout(parent, LV_LAYOUT_GRID);
			break;
		case LV_EVENT_PRESSING:
			;
			lv_point_t vect;
			lv_indev_get_vect(indev, &vect);

			lv_coord_t x = lv_obj_get_x(obj) + vect.x;
			lv_coord_t y = lv_obj_get_y(obj) + vect.y;
			lv_obj_set_pos(obj, x, y);
			break;
		case LV_EVENT_RELEASED: /* Cuando se suelta el objeto */
			lv_obj_add_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
			lv_obj_add_flag(lv_tabview_get_content(screen), LV_OBJ_FLAG_SCROLLABLE);
			break;
    default:
        break;
    }
}
static void slider_event_cb(lv_event_t * e){
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * obj = lv_event_get_target(e);
	
   if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
       int32_t * s = lv_event_get_param(e);
       *s = LV_MAX(*s, 60);
   }
   else if(code == LV_EVENT_DRAW_TASK_ADDED) {
       lv_draw_task_t * draw_task = lv_event_get_param(e);
       if(draw_task == NULL || draw_task->type != LV_DRAW_TASK_TYPE_FILL) return;
       lv_draw_rect_dsc_t * draw_rect_dsc = draw_task->draw_dsc;

       if(draw_rect_dsc->base.part == LV_PART_KNOB && lv_obj_has_state(obj, LV_STATE_PRESSED)) {
           char buf[8];
           lv_snprintf(buf, sizeof(buf), "%"LV_PRId32, lv_slider_get_value(obj));

           lv_point_t text_size;
           lv_text_get_size(&text_size, buf, font_normal, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

           lv_area_t txt_area;
           txt_area.x1 = draw_task->area.x1 + lv_area_get_width(&draw_task->area) / 2 - text_size.x / 2;
           txt_area.x2 = txt_area.x1 + text_size.x;
           txt_area.y2 = draw_task->area.y1 - 10;
           txt_area.y1 = txt_area.y2 - text_size.y;

           lv_area_t bg_area;
           bg_area.x1 = txt_area.x1 - LV_DPX(8);
           bg_area.x2 = txt_area.x2 + LV_DPX(8);
           bg_area.y1 = txt_area.y1 - LV_DPX(8);
           bg_area.y2 = txt_area.y2 + LV_DPX(8);

           lv_draw_rect_dsc_t rect_dsc;
           lv_draw_rect_dsc_init(&rect_dsc);
           rect_dsc.bg_color = lv_palette_darken(LV_PALETTE_GREY, 3);
           rect_dsc.radius = LV_DPX(5);
           lv_draw_rect(draw_rect_dsc->base.layer, &rect_dsc, &bg_area);

           lv_draw_label_dsc_t label_dsc;
           lv_draw_label_dsc_init(&label_dsc);
           label_dsc.color = lv_color_white();
           label_dsc.font = font_normal;
           label_dsc.text = buf;
           label_dsc.text_local = 1;
           lv_draw_label(draw_rect_dsc->base.layer, &label_dsc, &txt_area);
       }
   }
}
static void set_vol(lv_obj_t * slider, uint8_t vol){
	lv_anim_t a, b, c;
	lv_anim_init(&a); lv_anim_init(&b); lv_anim_init(&c);
	lv_anim_set_var(&a, volume_slider_radio); lv_anim_set_var(&b, volume_slider_mp3); lv_anim_set_var(&c, volume_slider_filtros);
	lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_slider_set_value); lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t) lv_slider_set_value); lv_anim_set_exec_cb(&c, (lv_anim_exec_xcb_t) lv_slider_set_value);
	int duracion_ms = 250;
	lv_anim_set_time(&a, duracion_ms); lv_anim_set_time(&b, duracion_ms); lv_anim_set_time(&c, duracion_ms);
	if(slider == volume_slider_mp3){
		lv_slider_set_value(volume_slider_radio, vol, LV_ANIM_OFF);
		lv_slider_set_value(volume_slider_filtros, vol, LV_ANIM_OFF);
	} else if(slider == volume_slider_radio){
		lv_slider_set_value(volume_slider_mp3, vol, LV_ANIM_OFF);
		lv_slider_set_value(volume_slider_filtros, vol, LV_ANIM_OFF);
	} else if(slider == volume_slider_filtros){
		lv_slider_set_value(volume_slider_mp3, vol, LV_ANIM_OFF);
		lv_slider_set_value(volume_slider_radio, vol, LV_ANIM_OFF);
	} else{ // Desde la web
		uint8_t current_volume = lv_slider_get_value(volume_slider_radio);
		lv_anim_set_values(&a, current_volume, vol); lv_anim_set_values(&b, current_volume, vol); lv_anim_set_values(&c, current_volume, vol);
		lv_anim_start(&a); lv_anim_start(&b); lv_anim_start(&c);
	}
}
static void volume_cb(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * obj = lv_event_get_target(e);

	uint8_t vol = lv_slider_get_value(obj);

	if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
		int32_t * s = lv_event_get_param(e);
		 *s = LV_MAX(*s, 60);
	}
	set_vol(obj, vol);
//	else if(code == LV_EVENT_DRAW_TASK_ADDED) {
//		lv_draw_task_t * draw_task = lv_event_get_param(e);
//		if(draw_task == NULL || draw_task->type != LV_DRAW_TASK_TYPE_FILL) return;
//			lv_draw_rect_dsc_t * draw_rect_dsc = draw_task->draw_dsc;

//		if(draw_rect_dsc->base.part == LV_PART_KNOB && lv_obj_has_state(obj, LV_STATE_PRESSED)) {
//				char buf[8];
//				lv_snprintf(buf, sizeof(buf), "%"LV_PRId32, lv_slider_get_value(obj));

//				lv_point_t text_size;
//				lv_text_get_size(&text_size, buf, font_normal, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

//				lv_area_t txt_area;
//				txt_area.x1 = draw_task->area.x1 + lv_area_get_width(&draw_task->area) / 2 - text_size.x / 2;
//				txt_area.x2 = txt_area.x1 + text_size.x;
//				txt_area.y2 = draw_task->area.y1 - 10;
//				txt_area.y1 = txt_area.y2 - text_size.y;

//				lv_area_t bg_area;
//				bg_area.x1 = txt_area.x1 - LV_DPX(8);
//				bg_area.x2 = txt_area.x2 + LV_DPX(8);
//				bg_area.y1 = txt_area.y1 - LV_DPX(8);
//				bg_area.y2 = txt_area.y2 + LV_DPX(8);

//				lv_draw_rect_dsc_t rect_dsc;
//				lv_draw_rect_dsc_init(&rect_dsc);
//				rect_dsc.bg_color = lv_palette_darken(LV_PALETTE_GREY, 3);
//				rect_dsc.radius = LV_DPX(5);
//				lv_draw_rect(draw_rect_dsc->base.layer, &rect_dsc, &bg_area);

//				lv_draw_label_dsc_t label_dsc;
//				lv_draw_label_dsc_init(&label_dsc);
//				label_dsc.color = lv_color_white();
//				label_dsc.font = font_normal;
//				label_dsc.text = buf;
//				label_dsc.text_local = 1;
//				lv_draw_label(draw_rect_dsc->base.layer, &label_dsc, &txt_area);
//			}
//   }
}
static void mute_cb(lv_event_t * e) {
	static uint8_t prev_volume;
	
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * btn = lv_event_get_target(e);
	lv_obj_t * panel = lv_obj_get_parent(btn);
	
	if(code == LV_EVENT_CLICKED) {
		int current_volume = lv_slider_get_value(volume_slider_radio);
		
		lv_anim_t a, b, c;
		lv_anim_init(&a); lv_anim_init(&b); lv_anim_init(&c);
		lv_anim_set_var(&a, volume_slider_radio); lv_anim_set_var(&b, volume_slider_mp3); lv_anim_set_var(&c, volume_slider_filtros);
		lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_slider_set_value); lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t) lv_slider_set_value); lv_anim_set_exec_cb(&c, (lv_anim_exec_xcb_t) lv_slider_set_value);
		int duracion_ms = 250;
		lv_anim_set_time(&a, duracion_ms); lv_anim_set_time(&b, duracion_ms); lv_anim_set_time(&c, duracion_ms);
		
		if(current_volume == 0) {
			lv_anim_set_values(&a, 0, prev_volume); lv_anim_set_values(&b, 0, prev_volume); lv_anim_set_values(&c, 0, prev_volume);
			lv_anim_start(&a); lv_anim_start(&b); lv_anim_start(&c);
		} else {
			prev_volume = current_volume;
			lv_anim_set_values(&a, current_volume, 0); lv_anim_set_values(&b, current_volume, 0); lv_anim_set_values(&c, current_volume, 0);
			lv_anim_start(&a); lv_anim_start(&b); lv_anim_start(&c);
		}
		lv_anim_start(&a); lv_anim_start(&b); lv_anim_start(&c);
	}
}
static void ta_event_cb(lv_event_t * e){
   lv_event_code_t code = lv_event_get_code(e);
   lv_obj_t * ta = lv_event_get_target(e);
   lv_obj_t * kb = lv_event_get_user_data(e);
   if(code == LV_EVENT_FOCUSED) {
       if(lv_indev_get_type(lv_indev_active()) != LV_INDEV_TYPE_KEYPAD) {
           lv_keyboard_set_textarea(kb, ta);
           lv_obj_set_style_max_height(kb, LV_HOR_RES * 2 / 3, 0);
           lv_obj_update_layout(tv);   /*Be sure the sizes are recalculated*/
           lv_obj_set_height(tv, LV_VER_RES - lv_obj_get_height(kb));
           lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);
           lv_obj_scroll_to_view_recursive(ta, LV_ANIM_OFF);
           lv_indev_wait_release(lv_event_get_param(e));
       }
   }
   else if(code == LV_EVENT_DEFOCUSED) {
       lv_keyboard_set_textarea(kb, NULL);
       lv_obj_set_height(tv, LV_VER_RES);
       lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
       lv_indev_reset(NULL, ta);
   }
   else if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
		lv_obj_set_height(tv, LV_VER_RES);
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
		lv_indev_reset(NULL, ta);   /*To forget the last clicked object to make it focusable again*/
		// Notificar al principal
		int freq = lv_slider_get_value(slider_freq);
		msg_to_main.lcd_msg.type    = LCD_RADIO_FREQ;
		msg_to_main.lcd_msg.payload = freq;
		osMessageQueuePut(ctrl_in_queue, &msg_to_main, NULL, 0);	
	}
}
static void tabview_delete_event_cb(lv_event_t * e){
   lv_event_code_t code = lv_event_get_code(e);

   if(code == LV_EVENT_DELETE) {
       lv_style_reset(&style_text_muted);
       lv_style_reset(&style_title);
       lv_style_reset(&style_icon);
       lv_style_reset(&style_bullet);
   }
}
static void crear_panel_titulo (lv_obj_t * container){
	lv_obj_t * name = lv_label_create(container);
	lv_label_set_text(name, TITLE);
	lv_obj_add_style(name, &style_title, 0);
	
	lv_obj_t * about = lv_label_create(container);
	lv_label_set_text(about, ABOUT);
	
	static int32_t grid_1_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_1_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

	lv_obj_set_grid_dsc_array(container, grid_1_col_dsc, grid_1_row_dsc);
	
	lv_obj_set_grid_cell(name, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(about,LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
}

static lv_style_t scale_consumo_section1_main_style;
static lv_style_t scale_consumo_section1_indicator_style;
static lv_style_t scale_consumo_section1_tick_style;
static lv_style_t scale_consumo_section2_main_style;
static lv_style_t scale_consumo_section2_indicator_style;
static lv_style_t scale_consumo_section2_tick_style;
static lv_style_t scale_consumo_section3_main_style;
static lv_style_t scale_consumo_section3_indicator_style;
static lv_style_t scale_consumo_section3_tick_style;

static void scale_consumo_delete_event_cb(lv_event_t * e){
   lv_event_code_t code = lv_event_get_code(e);

   if(code == LV_EVENT_DELETE) {
       lv_style_reset(&scale_consumo_section1_main_style);
       lv_style_reset(&scale_consumo_section1_indicator_style);
       lv_style_reset(&scale_consumo_section1_tick_style);
       lv_style_reset(&scale_consumo_section2_main_style);
       lv_style_reset(&scale_consumo_section2_indicator_style);
       lv_style_reset(&scale_consumo_section2_tick_style);
       lv_style_reset(&scale_consumo_section3_main_style);
       lv_style_reset(&scale_consumo_section3_indicator_style);
       lv_style_reset(&scale_consumo_section3_tick_style);
   }
}
static void scale_consumo_anim_cb(void * var, int32_t v){
   lv_obj_t * needle = lv_obj_get_child(var, 0);
   lv_scale_set_image_needle_value(var, needle, v);

//   lv_obj_t * label = lv_obj_get_child(var, 1);
//   lv_label_set_text_fmt(label, "%"LV_PRId32, v);
}
static void crear_panel_consumo (lv_obj_t * container){
	lv_obj_t * label_consumo_title = lv_label_create(container);
	lv_label_set_text(label_consumo_title, "Consumo");
	lv_obj_add_style(label_consumo_title, &style_subtitle, 0);
	
	label_consumo = lv_label_create(container);
	lv_label_set_text(label_consumo, "Consumo actual: ");
	lv_obj_set_layout(label_consumo, LV_LAYOUT_NONE);
	lv_obj_set_pos(label_consumo, 0, 220);
	
	/* Create a scale object */ // Para cambiar el valor: lv_scale_set_line_needle_value(scale_consumo, needle_consumo, 80, valor);
	scale_consumo = lv_scale_create(container);
	lv_scale_set_mode(scale_consumo, LV_SCALE_MODE_ROUND_INNER);
	lv_scale_set_post_draw(scale_consumo, true);
	lv_obj_set_width(scale_consumo, 250);
	lv_obj_update_layout(container);
	lv_obj_set_height(scale_consumo, 250);
	
	lv_scale_set_range(scale_consumo, 0, 3000);
	lv_scale_set_total_tick_count(scale_consumo, 25);    // Antes 17
	lv_scale_set_major_tick_every(scale_consumo, 4);
	lv_obj_set_style_length(scale_consumo, 10, LV_PART_ITEMS);
	lv_obj_set_style_length(scale_consumo, 20, LV_PART_INDICATOR);
	lv_scale_set_angle_range(scale_consumo, 225);
	lv_scale_set_rotation(scale_consumo, 180);
	 
	lv_style_init(&scale_consumo_section1_main_style);
	lv_style_set_arc_width(&scale_consumo_section1_main_style, 2);
	lv_style_set_arc_color(&scale_consumo_section1_main_style, lv_palette_main(LV_PALETTE_GREEN));

	lv_style_init(&scale_consumo_section1_indicator_style);
	lv_style_set_line_width(&scale_consumo_section1_indicator_style, 2);
	lv_style_set_line_color(&scale_consumo_section1_indicator_style, lv_palette_darken(LV_PALETTE_GREEN, 2));

	lv_style_init(&scale_consumo_section1_tick_style);
	lv_style_set_line_width(&scale_consumo_section1_tick_style, 1);
	lv_style_set_line_color(&scale_consumo_section1_tick_style, lv_palette_darken(LV_PALETTE_GREEN, 2));

	lv_style_init(&scale_consumo_section2_main_style);
	lv_style_set_arc_width(&scale_consumo_section2_main_style, 2);
	lv_style_set_arc_color(&scale_consumo_section2_main_style, lv_palette_main(LV_PALETTE_BLUE));

	lv_style_init(&scale_consumo_section2_indicator_style);
	lv_style_set_line_width(&scale_consumo_section2_indicator_style, 2);
	lv_style_set_line_color(&scale_consumo_section2_indicator_style, lv_palette_darken(LV_PALETTE_BLUE, 2));

	lv_style_init(&scale_consumo_section2_tick_style);
	lv_style_set_line_width(&scale_consumo_section2_tick_style, 1);
	lv_style_set_line_color(&scale_consumo_section2_tick_style, lv_palette_darken(LV_PALETTE_BLUE, 2));

	lv_style_init(&scale_consumo_section3_main_style);
	lv_style_set_arc_width(&scale_consumo_section3_main_style, 2);
	lv_style_set_arc_color(&scale_consumo_section3_main_style, lv_palette_main(LV_PALETTE_RED));

	lv_style_init(&scale_consumo_section3_indicator_style);
	lv_style_set_line_width(&scale_consumo_section3_indicator_style, 2);
	lv_style_set_line_color(&scale_consumo_section3_indicator_style, lv_palette_darken(LV_PALETTE_RED, 2));

	lv_style_init(&scale_consumo_section3_tick_style);
	lv_style_set_line_width(&scale_consumo_section3_tick_style, 1);
	lv_style_set_line_color(&scale_consumo_section3_tick_style, lv_palette_darken(LV_PALETTE_RED, 2));

	lv_obj_add_event_cb(scale_consumo, scale_consumo_delete_event_cb, LV_EVENT_DELETE, NULL);
	
	lv_scale_section_t * section;
	section = lv_scale_add_section(scale_consumo);
	lv_scale_section_set_range(section, 0, 750);
	lv_scale_section_set_style(section, LV_PART_MAIN, &scale_consumo_section1_main_style);
	lv_scale_section_set_style(section, LV_PART_INDICATOR, &scale_consumo_section1_indicator_style);
	lv_scale_section_set_style(section, LV_PART_ITEMS, &scale_consumo_section1_tick_style);
	section = lv_scale_add_section(scale_consumo);
	lv_scale_section_set_range(section, 750, 2250);
	lv_scale_section_set_style(section, LV_PART_MAIN, &scale_consumo_section2_main_style);
	lv_scale_section_set_style(section, LV_PART_INDICATOR, &scale_consumo_section2_indicator_style);
	lv_scale_section_set_style(section, LV_PART_ITEMS, &scale_consumo_section2_tick_style);

	section = lv_scale_add_section(scale_consumo);
	lv_scale_section_set_range(section, 2250, 3000);
	lv_scale_section_set_style(section, LV_PART_MAIN, &scale_consumo_section3_main_style);
	lv_scale_section_set_style(section, LV_PART_INDICATOR, &scale_consumo_section3_indicator_style);
	lv_scale_section_set_style(section, LV_PART_ITEMS, &scale_consumo_section3_tick_style);

	needle_consumo = lv_line_create(scale_consumo);

	lv_obj_set_style_line_width(needle_consumo, 5, 0);
	lv_obj_set_style_line_rounded(needle_consumo, true, 0);
	lv_obj_set_style_line_color(needle_consumo, lv_palette_main(LV_PALETTE_RED), 0);
	
	static const char * custom_labels[] = {"0 A", "0'5 A", "1 A", "1'5 A", "2 A", "2'5 A    ", "3 A", NULL};
	lv_scale_set_text_src(scale_consumo, custom_labels);
	
	static int32_t grid_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Widget    */
//			5,                /* Separador */
//			LV_GRID_CONTENT,  /* Label     */
//			5,                /* Separador */
//			LV_GRID_CONTENT,  /* Switch */
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(container, grid_col_dsc, grid_row_dsc);
	
	lv_obj_set_grid_cell(label_consumo_title, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
	lv_obj_set_grid_cell(scale_consumo, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
	
	lv_scale_set_line_needle_value(scale_consumo, needle_consumo, 80, 750);
}
static void crear_panel_freq (lv_obj_t * container){
	lv_obj_t * kb = lv_keyboard_create(lv_screen_active());
	lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
	lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
	
	lv_obj_t * label_freq_input = lv_label_create(container);
	lv_label_set_text(label_freq_input, "Sintonizar una frecuencia [MHz]");
	lv_obj_add_style(label_freq_input, &style_subtitle, 0);
	
	/* Textarea */
	textarea_freq = lv_textarea_create(container);
	lv_obj_set_width(textarea_freq, LV_PCT(100));
	lv_textarea_set_one_line(textarea_freq, true);
	lv_textarea_set_placeholder_text(textarea_freq, "87.0");
	lv_obj_add_event_cb(textarea_freq, ta_event_cb, LV_EVENT_ALL, kb);
  lv_obj_add_event_cb(textarea_freq, frequency_set_cb, LV_EVENT_VALUE_CHANGED, NULL);
//	lv_obj_add_event_cb(textarea_freq, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	
	static lv_style_t style_ta;
	lv_style_init(&style_ta);
	
	static lv_style_t style_cursor;
	lv_style_init(&style_cursor);
	
	lv_style_set_bg_color(&style_cursor, lv_color_black());

	lv_style_set_bg_opa(&style_ta, LV_OPA_TRANSP);  
	lv_style_set_border_width(&style_ta, 0); 
	
	lv_style_set_text_font(&style_ta, font_ultralarge); 
	lv_style_set_text_align(&style_ta, LV_TEXT_ALIGN_CENTER); 
	lv_style_set_text_opa(&style_ta, LV_OPA_100);
	
//	lv_obj_remove_style_all(textarea_freq);
	lv_obj_add_style(textarea_freq, &style_ta, LV_PART_MAIN);
	lv_obj_add_style(textarea_freq, &style_cursor, LV_PART_CURSOR);
	
	/* Scale */
	lv_obj_t * scale_freq = lv_scale_create(container);
	
	lv_scale_set_post_draw(scale_freq, true);
	
	lv_scale_set_range(scale_freq, 87, 108);
	lv_scale_set_total_tick_count(scale_freq,43);
	lv_scale_set_major_tick_every(scale_freq, 6);
	lv_obj_set_style_length(scale_freq, 10, LV_PART_ITEMS);
	lv_obj_set_style_length(scale_freq, 20, LV_PART_INDICATOR);
	lv_obj_set_height(scale_freq, 35);
	
	/* Slider */
	slider_freq = lv_slider_create(container);
	
	lv_slider_set_range(slider_freq, 87 * 10, 108 * 10); // Centenas de kHz
	
	static lv_style_t *style_knob;
	lv_style_init(style_knob);
	lv_style_set_bg_opa(style_knob, LV_OPA_COVER);
	lv_style_set_bg_color(style_knob, lv_palette_main(LV_PALETTE_RED));
	lv_style_set_radius(style_knob, LV_RADIUS_CIRCLE);
	lv_style_set_border_width(style_knob, 2);
	lv_style_set_pad_all(style_knob, 2);

	lv_obj_remove_style_all(slider_freq); 
	lv_obj_add_style(slider_freq, style_knob, LV_PART_KNOB);	
	
	lv_obj_add_event_cb(slider_freq, frequency_set_cb, LV_EVENT_ALL, NULL);
	
	/* Label "cadena" */
	label_cadena_txt = lv_label_create(container);
	lv_label_set_text(label_cadena_txt, "");
	lv_obj_add_style(label_cadena_txt, &style_title, 0);
	
	/* Cadenas guardadas */
	list_cadenas = lv_list_create(container);
	
	/* Seek up / down & save */
	lv_obj_t * seek_up_btn = lv_btn_create(container);
	lv_obj_add_event_cb(seek_up_btn, seek_up_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_seek_up = lv_label_create(seek_up_btn);
	lv_label_set_text_fmt(label_seek_up, "%s Seek Up", LV_SYMBOL_UP);
	
	lv_obj_t * seek_down_btn = lv_btn_create(container);
	lv_obj_add_event_cb(seek_down_btn, seek_down_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_seek_down = lv_label_create(seek_down_btn);
	lv_label_set_text_fmt(label_seek_down, "%s Seek Down", LV_SYMBOL_DOWN);
	
	lv_obj_t * guardar_cadena_btn = lv_btn_create(container);
	lv_obj_add_event_cb(guardar_cadena_btn, guardar_cadena_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_guardar_cadena = lv_label_create(guardar_cadena_btn);
	lv_label_set_text_fmt(label_guardar_cadena, "%s Favoritos", LV_SYMBOL_DOWNLOAD);
	
	static int32_t grid_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
			LV_GRID_CONTENT,  /* Freq      */
			LV_GRID_CONTENT,  /* Scale     */
			LV_GRID_CONTENT,  /* Cadena    */
			LV_GRID_CONTENT,  /* seek up   */
			LV_GRID_CONTENT,  /* seek down */
			LV_GRID_CONTENT,  /* guardar   */
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(container, grid_col_dsc, grid_row_dsc);
	
	lv_obj_set_grid_cell(label_freq_input,   LV_GRID_ALIGN_CENTER,  0, 2, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(textarea_freq,      LV_GRID_ALIGN_CENTER,  0, 2, LV_GRID_ALIGN_STRETCH, 1, 1);
	lv_obj_set_grid_cell(scale_freq,         LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_START,   2, 1);
	lv_obj_set_grid_cell(slider_freq,        LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_START,   2, 1);
	lv_obj_set_grid_cell(label_cadena_txt,   LV_GRID_ALIGN_CENTER,  0, 2, LV_GRID_ALIGN_STRETCH, 3, 1);
	lv_obj_set_grid_cell(seek_up_btn,        LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START,   4, 1);
	lv_obj_set_grid_cell(seek_down_btn,      LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START,   5, 1);
	lv_obj_set_grid_cell(guardar_cadena_btn, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START,   6, 1);
	lv_obj_set_grid_cell(list_cadenas,       LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 4, 3);
}
static void crear_panel_mp3 (lv_obj_t * container){
	label_title_mp3 = lv_label_create(container);
	label_cancion = lv_label_create(container);
	lv_label_set_text(label_title_mp3, "Reproductor MP3");
	lv_obj_add_style(label_title_mp3, &style_title, 0);  // Make font bigger
	lv_label_set_text(label_cancion, "");
	lv_obj_add_style(label_cancion, &style_title, 0);
	
	lv_obj_t * panel_snaps = lv_obj_create(container);
	crear_panel_snaps_mp3(panel_snaps);	
	
	static int32_t grid_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_row_dsc[] = {
			LV_GRID_CONTENT,
			LV_GRID_FR(1),
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(container, grid_col_dsc, grid_row_dsc);
	
	lv_obj_set_grid_cell(label_title_mp3, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
	lv_obj_set_grid_cell(label_cancion, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
	
	lv_obj_set_grid_cell(panel_snaps, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
}

static void scroll_event_cb(lv_event_t * e){
	lv_obj_t * obj = lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);

	lv_obj_t * first_child = lv_obj_get_child(obj, NULL);
	
	lv_point_t p;
  lv_indev_get_vect(lv_indev_get_act(), &p);
	
	if(code == LV_EVENT_SCROLL){
		lv_dir_t dir = lv_indev_get_scroll_dir(lv_indev_get_act());
		
		if(p.y < 0){
			lv_coord_t new_height = 0;
			lv_obj_set_height(first_child, new_height);
		} else if(p.y > 0){
			lv_coord_t new_height = 100000;
			lv_obj_set_height(first_child, new_height);
		}
	}
}


static void crear_panel_snaps_mp3 (lv_obj_t * container){
	lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_align(container, LV_FLEX_ALIGN_CENTER);
	
	lv_style_init(&style_ctrl_mp3);
	lv_style_set_min_height(&style_ctrl_mp3, lv_pct(45));
	lv_style_set_max_height(&style_ctrl_mp3, lv_pct(100));
	lv_style_set_height(&style_ctrl_mp3, lv_pct(100));
	lv_style_set_width(&style_ctrl_mp3, lv_pct(100));
	lv_style_set_border_opa(&style_ctrl_mp3, 0);
	
	panel_ctrl_mp3 = lv_obj_create(container);
	crear_panel_control_mp3(panel_ctrl_mp3);
	lv_obj_add_style(panel_ctrl_mp3, &style_ctrl_mp3, 0);
	lv_obj_remove_flag(panel_ctrl_mp3, LV_OBJ_FLAG_SCROLLABLE);
	
	static lv_style_t style_list, style_btn;
	lv_style_init(&style_list); lv_style_init(&style_btn);
	lv_style_set_height(&style_list, lv_pct(65));
	lv_style_set_width(&style_list, lv_pct(100));
	lv_style_set_bg_color(&style_list, lv_palette_lighten(LV_PALETTE_GREY, 4));
	lv_style_set_bg_color(&style_btn,  lv_palette_lighten(LV_PALETTE_GREY, 4));
	
	lv_obj_t * list_canciones = lv_list_create(container);
	
	for (int i = 0; i < data.num_canciones; i ++){
		lv_obj_t * cancion = lv_list_add_btn(list_canciones, LV_SYMBOL_AUDIO, data.songs[i]);
		lv_obj_add_event_cb(cancion, mp3_list_cb, LV_EVENT_CLICKED, NULL);
		lv_obj_add_style(cancion, &style_btn, 0);
	}
	lv_obj_set_width(list_canciones, lv_pct(100));
	lv_obj_add_style(list_canciones, &style_list, 0);
	
	lv_obj_add_event_cb(container, scroll_event_cb, LV_EVENT_SCROLL, NULL);
}
static void crear_panel_control_mp3(lv_obj_t * container){
	static lv_style_t style_play;
	lv_style_init(&style_play);
	

	lv_style_set_radius(&style_play, LV_RADIUS_CIRCLE);
	lv_style_set_bg_grad_color(&style_play, lv_palette_lighten(LV_PALETTE_PINK, 2));
	lv_style_set_bg_grad_dir(&style_play, LV_GRAD_DIR_VER);

	btn_play_mp3 = lv_btn_create(container);
	lv_obj_add_style(btn_play_mp3, &style_play, LV_PART_MAIN);
	lv_obj_add_event_cb(btn_play_mp3, mp3_ctrl_cb, LV_EVENT_CLICKED, NULL);
	lv_style_set_pad_all(&style_play, 30);          // Make btn bigger
	
	lv_obj_t * label_play = lv_label_create(btn_play_mp3);
	lv_label_set_text(label_play, LV_SYMBOL_PLAY " " LV_SYMBOL_PAUSE);
	lv_obj_add_style(label_play, &style_title, 0);  // Make font bigger
	
	static lv_style_t style_next_prev;
	lv_style_init(&style_next_prev);

	lv_style_set_radius(&style_next_prev, LV_RADIUS_CIRCLE);
	lv_style_set_bg_grad_color(&style_next_prev, lv_palette_lighten(LV_PALETTE_PINK, 2));
	lv_style_set_bg_grad_dir(&style_next_prev, LV_GRAD_DIR_VER);

	btn_prev_mp3 = lv_btn_create(container);
	lv_obj_add_style(btn_prev_mp3, &style_next_prev, LV_PART_MAIN);
	lv_style_set_pad_all(&style_next_prev, 25);          // Make btn bigger
	
	lv_obj_t * label_prev = lv_label_create(btn_prev_mp3);
	lv_label_set_text(label_prev, " " LV_SYMBOL_PREV " ");
	lv_obj_add_event_cb(btn_prev_mp3, mp3_ctrl_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_style(label_prev, &style_subtitle, 0);  // Make font bigger
	
	btn_next_mp3 = lv_btn_create(container);
	lv_obj_add_style(btn_next_mp3, &style_next_prev, LV_PART_MAIN);
	lv_style_set_pad_all(&style_next_prev, 20);          // Make btn bigger
	
	lv_obj_t * label_next = lv_label_create(btn_next_mp3);
	lv_label_set_text(label_next, " " LV_SYMBOL_NEXT " ");
	lv_obj_add_event_cb(btn_next_mp3, mp3_ctrl_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_style(label_next, &style_subtitle, 0);  // Make font bigger
	
	static int32_t grid_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_row_dsc[] = {
			LV_GRID_FR(1),
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(container, grid_col_dsc, grid_row_dsc);
	
	lv_obj_set_grid_cell(btn_prev_mp3, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
	lv_obj_set_grid_cell(btn_play_mp3, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
	lv_obj_set_grid_cell(btn_next_mp3, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
}

static void crear_panel_canciones   (lv_obj_t * container){
	lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
	
	lv_obj_t * list_canciones = lv_list_create(container);
	
	for (int i = 0; i < 5; i ++){
		lv_list_add_btn(list_canciones, NULL, "AAAAA");
	}
	lv_obj_set_width(list_canciones, lv_pct(100));
}
static void crear_panel_seek (lv_obj_t * container){
	/* Create seek up and down buttons */
	lv_obj_t * label_seek = lv_label_create(container);
//		lv_obj_add_style(label_seek, &style_title, 0);
	lv_label_set_text_fmt(label_seek, "Buscar emisora");
	lv_obj_add_style(label_seek, &style_subtitle, 0);  
	
	lv_obj_t * seek_up_btn = lv_btn_create(container);
	lv_obj_add_event_cb(seek_up_btn, seek_up_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_seek_up = lv_label_create(seek_up_btn);
	lv_label_set_text_fmt(label_seek_up, "Up");
	
	lv_obj_t * seek_down_btn = lv_btn_create(container);
	lv_obj_add_event_cb(seek_down_btn, seek_down_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_seek_down = lv_label_create(seek_down_btn);
	lv_label_set_text_fmt(label_seek_down, "Down");
	
	static int32_t grid_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Up        */
			LV_GRID_CONTENT,  /* Down      */
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(container, grid_col_dsc, grid_row_dsc);
	
	lv_obj_set_grid_cell(label_seek,    LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(seek_up_btn,   LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(seek_down_btn, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 3, 1);
}
static void crear_panel_salida (lv_obj_t * container, lv_obj_t * btn_headphones, lv_obj_t * btn_speakers){
	lv_obj_t * label_salida = lv_label_create(container);
	lv_label_set_text_fmt(label_salida, "Salida");
	lv_obj_add_style(label_salida, &style_subtitle, 0);  
	
	lv_obj_add_event_cb(btn_headphones, headphones_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_headphones = lv_label_create(btn_headphones);
	lv_label_set_text_fmt(label_headphones, "Cascos");
	lv_obj_add_style(btn_headphones, &btn_style_sel, 0);
	
	lv_obj_add_event_cb(btn_speakers, speakers_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_altavoz = lv_label_create(btn_speakers);
	lv_label_set_text_fmt(label_altavoz, "Altavoz");
	lv_obj_add_style(btn_speakers, &btn_style_base, 0);

	static int32_t grid_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Up        */
			LV_GRID_CONTENT,  /* Down      */
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(container, grid_col_dsc, grid_row_dsc);
	
	lv_obj_set_grid_cell(label_salida,   LV_GRID_ALIGN_CENTER,  0, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(btn_headphones, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(btn_speakers,   LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 3, 1);
}
static void crear_panel_volumen (lv_obj_t * container, lv_obj_t * vol_slider){
/* No funciona. Quizas pasando un boton mute como parametro, haciendo mutes globales o estáticos? */	
	
//	label_volumen = lv_label_create(container);
//	lv_label_set_text_fmt(label_volumen, "Volumen");
//	lv_obj_add_style(label_volumen, &style_subtitle, 0);  
//	
//	vol_slider = lv_slider_create(container);
//	lv_obj_add_event_cb(vol_slider, volume_cb, LV_EVENT_VALUE_CHANGED, NULL);
//	lv_slider_set_value(vol_slider, data.vol, LV_ANIM_ON);	
////	lv_slider_set_range(volume_slider_filtros, 0, 100);
//	
//	lv_obj_t * mute_btn = lv_btn_create(container);
//	lv_obj_add_event_cb(mute_btn, mute_cb, LV_EVENT_CLICKED, NULL);
//	lv_obj_t * label_play_btn = lv_label_create(mute_btn);
//	lv_label_set_text_fmt(label_play_btn, "%s  Mute", LV_SYMBOL_MUTE);
//	
//	static int32_t grid_vol_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
//	static int32_t grid_vol_row_dsc[] = {
//			LV_GRID_CONTENT,  /* Subtitulo */
//			5,                /* Separador */
//			LV_GRID_CONTENT,  /* Slider    */
//			5,                /* Separador */
//			LV_GRID_CONTENT,  /* Mute      */
//			LV_GRID_TEMPLATE_LAST
//	};
//	
//	lv_obj_set_grid_dsc_array(container, grid_vol_col_dsc, grid_vol_row_dsc);
//	
//	lv_obj_set_grid_cell(label_volumen, LV_GRID_ALIGN_CENTER,  0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
//	lv_obj_set_grid_cell(vol_slider,    LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
//	lv_obj_set_grid_cell(mute_btn,      LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 4, 1);
}
static void crear_panel_camino_audio(lv_obj_t * container){
	/* Botones salida de audio */
	lv_obj_t * label_salida = lv_label_create(container);
	lv_label_set_text_fmt(label_salida, "Salida audio");
  lv_obj_add_style(label_salida, &style_subtitle, 0);  
	
	btn_headphones_filtros = lv_btn_create(container);
	lv_obj_add_event_cb(btn_headphones_filtros, headphones_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_headphones = lv_label_create(btn_headphones_filtros);
	lv_label_set_text_fmt(label_headphones, "Cascos");
	lv_obj_add_style(btn_headphones_filtros, &btn_style_sel,  0);
		
	btn_speakers_filtros = lv_btn_create(container);
	lv_obj_add_event_cb(btn_speakers_filtros, speakers_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_altavoz = lv_label_create(btn_speakers_filtros);
	lv_label_set_text_fmt(label_altavoz, "Altavoz");
	lv_obj_add_style(btn_speakers_filtros, &btn_style_base, 0);
	
	btn_mp3_filtros = lv_btn_create(container);
	lv_obj_add_event_cb(btn_mp3_filtros, mp3_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_mp3 = lv_label_create(btn_mp3_filtros);
	lv_label_set_text_fmt(label_mp3, "MP3");
	lv_obj_add_style(btn_mp3_filtros, &btn_style_sel,  0);
		
	btn_radio_filtros = lv_btn_create(container);
	lv_obj_add_event_cb(btn_radio_filtros, radio_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_radio = lv_label_create(btn_radio_filtros);
	lv_label_set_text_fmt(label_radio, "Radio");
	lv_obj_add_style(btn_radio_filtros, &btn_style_base, 0);
	
	static int32_t grid_sal_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_sal_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Boton Cascos */
			LV_GRID_CONTENT,  /* Boton Altavoz */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Boton MP3 */
			LV_GRID_CONTENT,  /* Boton radio */
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(container, grid_sal_col_dsc, grid_sal_row_dsc);
	
	lv_obj_set_grid_cell(label_salida,   LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(btn_headphones_filtros, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(btn_speakers_filtros,   LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 3, 1);
	lv_obj_set_grid_cell(btn_mp3_filtros,        LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 5, 1);
	lv_obj_set_grid_cell(btn_radio_filtros,      LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 6, 1);
}
static void crear_panel_config_rapida(lv_obj_t * container){
	/* Botones salida de audio */
	lv_obj_t * label_salida = lv_label_create(container);
	lv_label_set_text_fmt(label_salida, "Configuracion rapida");
  lv_obj_add_style(label_salida, &style_subtitle, 0);  
	
	btn_headphones_conf_rapida = lv_btn_create(container);
	lv_obj_add_event_cb(btn_headphones_conf_rapida, headphones_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_headphones = lv_label_create(btn_headphones_conf_rapida);
	lv_label_set_text_fmt(label_headphones, "Cascos");
	lv_obj_add_style(btn_headphones_conf_rapida, &btn_style_sel,  0);
	
	btn_speakers_conf_rapida = lv_btn_create(container);
	lv_obj_add_event_cb(btn_speakers_conf_rapida, speakers_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_altavoz = lv_label_create(btn_speakers_conf_rapida);
	lv_label_set_text_fmt(label_altavoz, "Altavoz");
	lv_obj_add_style(btn_speakers_conf_rapida, &btn_style_base, 0);
	
	btn_mp3_conf_rapida = lv_btn_create(container);
	lv_obj_add_event_cb(btn_mp3_conf_rapida, mp3_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_mp3 = lv_label_create(btn_mp3_conf_rapida);
	lv_label_set_text_fmt(label_mp3, "MP3");
	lv_obj_add_style(btn_mp3_conf_rapida,   &btn_style_sel,  0);
		
	btn_radio_conf_rapida = lv_btn_create(container);
	lv_obj_add_event_cb(btn_radio_conf_rapida, radio_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_radio = lv_label_create(btn_radio_conf_rapida);
	lv_label_set_text_fmt(label_radio, "Radio");
	lv_obj_add_style(btn_radio_conf_rapida, &btn_style_base, 0);
	
	lv_obj_t * btn_low_power = lv_btn_create(container);
	lv_obj_add_event_cb(btn_low_power, low_power_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_low_power = lv_label_create(btn_low_power);
	lv_label_set_text_fmt(label_low_power, "Bajo consumo");
	lv_obj_add_style(btn_low_power, &btn_style_base, 0);
	
	static int32_t grid_sal_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_sal_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Boton Cascos */
			LV_GRID_CONTENT,  /* Boton Altavoz */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Boton MP3 */
			LV_GRID_CONTENT,  /* Boton radio */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Boton bajo consumo */
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(container, grid_sal_col_dsc, grid_sal_row_dsc);
	
	lv_obj_set_grid_cell(label_salida,   LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(btn_headphones_conf_rapida, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(btn_speakers_conf_rapida,   LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 3, 1);
	lv_obj_set_grid_cell(btn_mp3_conf_rapida,        LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 5, 1);
	lv_obj_set_grid_cell(btn_radio_conf_rapida,      LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 6, 1);
	lv_obj_set_grid_cell(btn_low_power,  LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 8, 1);
}
static void set_freq(lv_obj_t * obj, uint16_t val){
	char buf[10] = "";
	if (obj == slider_freq){
		float freq = ((float)val)/10.0;
		sprintf(buf, "%.1f", freq);
		lv_textarea_set_text(textarea_freq, buf);
	} else if(obj == textarea_freq){
		lv_slider_set_value(slider_freq, val, LV_ANIM_ON);
	} else { // Desde la web
		lv_slider_set_value(slider_freq, val, LV_ANIM_ON);
// Cambiar el slider con animacion. Comentado pq hace un artefacto raro. Eliminar sentencia de arriba si se quiere probar
//		lv_anim_t a;
//		lv_anim_init(&a);
//		lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_slider_set_value);
//		int duracion_ms = 250;
//		lv_anim_set_time(&a, duracion_ms);
//		lv_anim_set_var(&a, slider_freq);
//		int current_value = lv_slider_get_value(slider_freq);
//		lv_anim_set_values(&a, 870, val);
//		lv_anim_start(&a);
		
		// Cambiar el numerin de arriba (textarea)
		float freq = ((float)val)/10.0;
		sprintf(buf, "%.1f", freq);
		lv_textarea_set_text(textarea_freq, buf);
	}
	
	// Buscar la cadena
	float freq = ((float)val)/10.0;
	
	char buf2[50] = "", buf3[50]= "";
	snprintf(buf2, 50, "");
	get_cadena(buf2, freq);
	if(strcmp(buf2, "") != 0){
		lv_label_set_text(label_cadena_txt, buf2);
	} else lv_label_set_text(label_cadena_txt, "");
}
static void frequency_set_cb(lv_event_t * e) {
	lv_obj_t * obj = lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);
	
	char buf[10] = "";
	uint16_t freq;
	if (obj == slider_freq){
		freq = lv_slider_get_value(slider_freq);
	} else if(obj == textarea_freq){
		sprintf(buf, "%s", lv_textarea_get_text(textarea_freq));
		float freq_f = atof(buf);
		freq = (freq_f*10.0); 
	}
	
	// Notificar al principal
	if(code == LV_EVENT_RELEASED){
		msg_to_main.lcd_msg.type    = LCD_RADIO_FREQ;
		msg_to_main.lcd_msg.payload = freq;
		osMessageQueuePut(ctrl_in_queue, &msg_to_main, NULL, 0);	
	}
	if(code == LV_EVENT_VALUE_CHANGED || code == LV_EVENT_RELEASED)
		set_freq(obj, freq);
}
static void guardar_cadena_cb(lv_event_t * e) {
	char buf2[50] = "";
	snprintf(buf2, 50, "");
	
	float freq = ((float)lv_slider_get_value(slider_freq))/10.0;
	get_cadena(buf2, freq);
	
	lv_obj_t * btn;
	
	if(strcmp(buf2, "") == 0){
		char tmp[10] = "";
		sprintf(tmp, "%.1f", freq);
		btn = lv_list_add_btn(list_cadenas, NULL, tmp);
	} else {
		btn = lv_list_add_btn(list_cadenas, NULL, lv_label_get_text(label_cadena_txt));
	}
	lv_obj_add_event_cb(btn, set_channel_from_list_cb, LV_EVENT_CLICKED, NULL);
	lista_freq_guardadas[index] = freq;
	index = ((index + 1)%15);
}
static void set_channel_from_list_cb(lv_event_t * e){
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * obj = lv_event_get_target(e);
	
	uint32_t pos = lv_obj_get_index(obj);
	float freq = lista_freq_guardadas[pos];
	
	char buf[10] = "";
	sprintf(buf, "%.1f", freq);
	lv_textarea_set_text(textarea_freq, buf);
	int val = (freq*10.0); 
	lv_slider_set_value(slider_freq, val, LV_ANIM_ON);
	
	//Notificar al principal
	msg_to_main.lcd_msg.type    = LCD_RADIO_FREQ;
	msg_to_main.lcd_msg.payload = val;
	osMessageQueuePut(ctrl_in_queue, &msg_to_main, NULL, 0);	
}

static void cambiar_estilo_salida(int out){
	if(out == 0){
		lv_obj_add_style(btn_headphones_conf_rapida, &btn_style_sel,  0);
		lv_obj_add_style(btn_headphones_radio,       &btn_style_sel,  0);
		lv_obj_add_style(btn_headphones_mp3,         &btn_style_sel,  0);
		lv_obj_add_style(btn_headphones_filtros,     &btn_style_sel,  0);
		lv_obj_add_style(btn_speakers_conf_rapida,   &btn_style_base, 0);
		lv_obj_add_style(btn_speakers_radio,         &btn_style_base, 0);
		lv_obj_add_style(btn_speakers_mp3,           &btn_style_base, 0);
		lv_obj_add_style(btn_speakers_filtros,       &btn_style_base, 0);
	} else {
		lv_obj_add_style(btn_headphones_conf_rapida, &btn_style_base, 0);
		lv_obj_add_style(btn_headphones_radio,       &btn_style_base, 0);
		lv_obj_add_style(btn_headphones_mp3,         &btn_style_base, 0);
		lv_obj_add_style(btn_headphones_filtros,     &btn_style_base, 0);
		lv_obj_add_style(btn_speakers_conf_rapida,   &btn_style_sel,  0);
		lv_obj_add_style(btn_speakers_radio,         &btn_style_sel,  0);
		lv_obj_add_style(btn_speakers_mp3,           &btn_style_sel,  0);
		lv_obj_add_style(btn_speakers_filtros,       &btn_style_sel,  0);
	}
}

static void headphones_cb(lv_event_t * e) {
	lv_obj_t * btn = lv_event_get_target(e);
	
	// Enviar mensaje al thread principal
	msg_to_main.lcd_msg.type    = LCD_OUTPUT_SEL;
	msg_to_main.lcd_msg.payload = 0;
	osMessageQueuePut(ctrl_in_queue, &msg_to_main, NULL, 0);
	
	// Cambios de estilo
	cambiar_estilo_salida(0);
}
static void speakers_cb(lv_event_t * e) {
	lv_obj_t * btn = lv_event_get_target(e);
	
	// Enviar mensaje al thread principal
	msg_to_main.lcd_msg.type    = LCD_OUTPUT_SEL;
	msg_to_main.lcd_msg.payload = 1;
	osMessageQueuePut(ctrl_in_queue, &msg_to_main, NULL, 0);
	
	// Cambios de estilo
	cambiar_estilo_salida(1);
}

static void cambiar_estilo_entrada(int in) {
	if(in == 0){
		lv_obj_add_style(btn_mp3_conf_rapida,   &btn_style_base, 0);
		lv_obj_add_style(btn_mp3_filtros,       &btn_style_base, 0);
		lv_obj_add_style(btn_radio_conf_rapida, &btn_style_sel,  0);
		lv_obj_add_style(btn_radio_filtros,     &btn_style_sel,  0);
	} else{
		lv_obj_add_style(btn_mp3_conf_rapida,   &btn_style_sel,  0);
		lv_obj_add_style(btn_mp3_filtros,       &btn_style_sel,  0);
		lv_obj_add_style(btn_radio_conf_rapida, &btn_style_base, 0);
		lv_obj_add_style(btn_radio_filtros,     &btn_style_base, 0);
	}
}

static void mp3_cb(lv_event_t * e){
	lv_obj_t * btn = lv_event_get_target(e);
	
	// Enviar mensaje al thread principal
	msg_to_main.lcd_msg.type    = LCD_INPUT_SEL;
	msg_to_main.lcd_msg.payload = 1;
	osMessageQueuePut(ctrl_in_queue, &msg_to_main, NULL, 0);
	
	// Cambios de estilo
	cambiar_estilo_entrada(1);
}
void restaurar_titulo_mp3(){
	lv_obj_fade_out(label_cancion, 300, 0);
	lv_obj_fade_in(label_title_mp3, 300, 300);
}
static void mp3_list_cb(lv_event_t * e){
	lv_obj_t * btn = lv_event_get_target(e);
	lv_obj_t * list = lv_obj_get_parent(btn);
	
	// Notificar al principal
	msg_to_main.lcd_msg.type = LCD_SONG;
	for(int i = 0; i < lv_obj_get_child_cnt(list); i ++){
		lv_obj_t* child = lv_obj_get_child(list, i);
		if(child == btn){
			msg_to_main.lcd_msg.payload = i+1;
			break;
		} 
	}
	osMessageQueuePut(ctrl_in_queue, &msg_to_main, NULL, 0);
	
	// Un efecto chulo
	lv_obj_fade_out(label_title_mp3, 300, 0);
	lv_label_set_text_fmt(label_cancion, "%s", lv_list_get_btn_text(list, btn));
	lv_obj_fade_in(label_cancion, 300, 300);
	lv_timer_t * tim = lv_timer_create(restaurar_titulo_mp3, 2500, NULL); // Tenemos que hacerlo con un timer, con varios delays solo se aplica la ultima animacion
	lv_timer_set_repeat_count(tim, 1);
}
static void mp3_ctrl_cb(lv_event_t * e){
	lv_obj_t * btn = lv_event_get_target(e);
	
	if(btn == btn_prev_mp3)
			msg_to_main.lcd_msg.type = LCD_PREV_SONG;
	else if (btn == btn_play_mp3)
			msg_to_main.lcd_msg.type = LCD_PLAY_PAUSE;
	else if (btn == btn_next_mp3)
			msg_to_main.lcd_msg.type = LCD_NEXT_SONG;
	
	osMessageQueuePut(ctrl_in_queue, &msg_to_main, NULL, 0);
}
static void seek_down_cb(lv_event_t * e) {
	// Notificar al principal
	msg_to_main.lcd_msg.type = LCD_SEEK;
	msg_to_main.lcd_msg.payload = 0;
	osMessageQueuePut(ctrl_in_queue, &msg_to_main, NULL, 0);
}
static void seek_up_cb(lv_event_t * e){
	// Notificar al principal
	msg_to_main.lcd_msg.type = LCD_SEEK;
	msg_to_main.lcd_msg.payload = 1;
	osMessageQueuePut(ctrl_in_queue, &msg_to_main, NULL, 0);
}
static void radio_cb(lv_event_t * e){
	lv_obj_t * btn = lv_event_get_target(e);
	
	// Enviar mensaje al thread principal
	msg_to_main.lcd_msg.type    = LCD_INPUT_SEL;
	msg_to_main.lcd_msg.payload = 0;
	osMessageQueuePut(ctrl_in_queue, &msg_to_main, NULL, 0);
	
	// Cambios de estilo
	cambiar_estilo_entrada(0);
}
static void set_filter(uint8_t band, int8_t value){
	// Solo accesible desde la web
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_slider_set_value);
	int duracion_ms = 250;
	lv_anim_set_time(&a, duracion_ms);
	int8_t current_value = 0;
	switch(band){
		case 0:
			lv_anim_set_var(&a, slider_b1);
			current_value = lv_slider_get_value(slider_b1);
		break;
		case 1:
			lv_anim_set_var(&a, slider_b2);
			current_value = lv_slider_get_value(slider_b2);
		break;
		case 2:
			lv_anim_set_var(&a, slider_b3);
			current_value = lv_slider_get_value(slider_b3);
		break;
		case 3:
			lv_anim_set_var(&a, slider_b4);
			current_value = lv_slider_get_value(slider_b4);
		break;
		case 4:
			lv_anim_set_var(&a, slider_b5);
			current_value = lv_slider_get_value(slider_b5);
		break;
		default:
		break;
	}
	lv_anim_set_values(&a, current_value, value);
	lv_anim_start(&a);
}
static void filter_cb(lv_event_t * e){
	// Esta callback solo salta cuando se suelta el slider.
	lv_obj_t * slider = lv_event_get_target(e);
	int current_value = lv_slider_get_value(slider);
	
	msg_to_main.lcd_msg.type    = LCD_BANDS;
	
	if(slider == slider_b1){
		msg_to_main.lcd_msg.payload = (0 << 8) | (current_value & 0xFF);
	} else if(slider == slider_b2){
		msg_to_main.lcd_msg.payload = (1 << 8) | (current_value & 0xFF);
	} else if(slider == slider_b3){
		msg_to_main.lcd_msg.payload = (2 << 8) | (current_value & 0xFF);
	} else if(slider == slider_b4){
		msg_to_main.lcd_msg.payload = (3 << 8) | (current_value & 0xFF);
	} else if(slider == slider_b5){
		msg_to_main.lcd_msg.payload = (4 << 8) | (current_value & 0xFF);
	}
	osMessageQueuePut(ctrl_in_queue, &msg_to_main, NULL, 0);
}
