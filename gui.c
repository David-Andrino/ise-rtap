#include "gui.h"
#include "stdio.h"
#include "math.h"

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN && LV_MEM_SIZE < (38ul * 1024ul)
    #error Insufficient memory. Please set LV_MEM_SIZE to at least 38KB (38ul * 1024ul).  48KB is recommended.
#endif

#define TITLE "Real Time Audio Processor"
#define ABOUT "Ingenieria de Sistemas Electronicos 2024"
#define DRAGGABBLE 0

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
} disp_size_t;

lv_obj_t * scale_consumo; 	// Tiene que ser global para modificarlo desde async_cb
static lv_obj_t *needle_consumo, *needle_freq;
lv_obj_t * label_consumo;

/**********************
 *      Coeficientes
 **********************/
static lv_obj_t * label_coef;
void async_cb(void * data){
    char str[32]; /* Buffer to hold the string */
		int *n = ((int*) data);
    /* Format the new string. Replace this with your actual calculation result */
    sprintf(str, "Coeficiente %d: %d", n[0], n[1]);

    /* Set the new text */
    lv_label_set_text(label_coef, str);
}
void consumo_async_cb(void * data){
	char str[32]; /* Buffer to hold the string */
	int *n = ((int*) data);
	int consumo = abs(n[1] % 2000);
	/* Format the new string. Replace this with your actual calculation result */
	sprintf(str, "Consumo actual %d mA", consumo);

	/* Set the new text */
	lv_label_set_text(label_consumo, str);
	lv_scale_set_line_needle_value(scale_consumo, needle_consumo, 80, consumo);
}
/**********************
 *  STATIC PROTOTYPES
 **********************/
static void home_create(lv_obj_t * parent);
static void create_radio_content(lv_obj_t * tabview);
static void create_mp3_content(lv_obj_t * tabview);
static void create_filters_content(lv_obj_t * tabview);

static void crear_panel_titulo      (lv_obj_t * container);
static void crear_panel_consumo     (lv_obj_t * container);
static void crear_panel_freq        (lv_obj_t * container);
static void crear_panel_seek 				(lv_obj_t * container);
static void crear_panel_salida 			(lv_obj_t * container);
static void crear_panel_camino_audio(lv_obj_t * container);
static void crear_panel_volumen     (lv_obj_t * container, lv_obj_t * vol_slider);

static void ta_event_cb(lv_event_t * e);
static void slider_event_cb(lv_event_t * e);
static void tabview_delete_event_cb(lv_event_t * e);
static void freq_slider_observer_cb(lv_observer_t * observer, lv_subject_t * subject);

static void drag_event_handler(lv_event_t * e);
static void frequency_set_cb(lv_event_t * e);
static void seek_up_cb(lv_event_t * e) {}
static void seek_down_cb(lv_event_t * e) {}
static void volume_cb(lv_event_t * e);
static void headphones_cb(lv_event_t * e) {}
static void speakers_cb(lv_event_t * e) {}
static void mute_cb(lv_event_t * e);
static void save_cb(lv_event_t * e){}
static void mp3_cb(lv_event_t * e){}
static void radio_cb(lv_event_t * e){}

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

static const lv_font_t * font_ultralarge;
static const lv_font_t * font_large;
static const lv_font_t * font_medium;
static const lv_font_t * font_normal;

static lv_obj_t * label_volumen;
static lv_obj_t *volume_slider_radio, *volume_slider_mp3, *volume_slider_filtros;		// Necesario que sea global para poder copiar el valor. No puede haber solo un objeto compartido porque falla (Necesario uno por tab).

static lv_obj_t * textarea_freq;
static lv_obj_t * slider_freq;
static lv_obj_t * label_cadena;

static gui_data_t data = {
	.vol = 50,
	.songs = "Song 1\nSong 2\nSong 3\nSong 4\nSong 5",
	.fcentral[0] = "20Hz",
	.fcentral[1] = "110Hz",
	.fcentral[2] = "630Hz",
	.fcentral[3] = "3.5kHz",
	.fcentral[4] = "15kHz"
};

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

	lv_obj_t * t1 = lv_tabview_add_tab(tv, "Home");
	lv_obj_t * t2 = lv_tabview_add_tab(tv, "Radio");
	lv_obj_t * t3 = lv_tabview_add_tab(tv, "MP3");
	lv_obj_t * t4 = lv_tabview_add_tab(tv, "Filtros");
	
	lv_obj_t * tab_bar = lv_tabview_get_tab_bar(tv);
	lv_obj_set_style_pad_left(tab_bar, LV_HOR_RES / 2, 0);
	lv_obj_t * logo = lv_image_create(tab_bar);
	lv_obj_add_flag(logo, LV_OBJ_FLAG_IGNORE_LAYOUT);
//        LV_IMAGE_DECLARE(img_lvgl_logo);
//        lv_image_set_src(logo, &img_lvgl_logo);
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
	
	/* Camino de audio */
	lv_obj_t * panel_salida = lv_obj_create(tabview);
	lv_obj_set_height(panel_salida, LV_SIZE_CONTENT);
	lv_obj_set_width(panel_salida, LV_SIZE_CONTENT);
	crear_panel_camino_audio(panel_salida);
	
//	/Create a keyboard /
//	lv_obj_t * kb = lv_keyboard_create(lv_screen_active());
//	lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
//	lv_obj_add_event_cb(user_name, ta_event_cb, LV_EVENT_ALL, kb);
 
	/*Create the third panel*/
	lv_obj_t * panel_consumo = lv_obj_create(tabview);
  crear_panel_consumo(panel_consumo);
		
//	lv_obj_t * label_bajo_consumo = lv_label_create(panel_consumo);
//	lv_label_set_text(label_bajo_consumo, "Bajo consumo");
//	lv_obj_add_style(label_bajo_consumo, &style_text_muted, 0);

//	lv_obj_t * sw1 = lv_switch_create(panel_consumo);
		
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
	crear_panel_freq(panel_freq);
	
//	lv_obj_t * panel_seek = lv_obj_create(tabview);
//	lv_obj_set_height(panel_seek, LV_SIZE_CONTENT);
//	lv_obj_set_width(panel_seek , LV_SIZE_CONTENT);
//	crear_panel_seek(panel_seek);
	
	/* Volumen */
	lv_obj_t * panel_vol = lv_obj_create(tabview);
	lv_obj_set_height(panel_vol, LV_SIZE_CONTENT);
	lv_obj_set_width(panel_vol , LV_SIZE_CONTENT);
	label_volumen = lv_label_create(panel_vol);
	lv_label_set_text_fmt(label_volumen, "Volumen");
	lv_obj_add_style(label_volumen, &style_subtitle, 0);  
	
	volume_slider_radio = lv_slider_create(panel_vol);
	lv_obj_add_event_cb(volume_slider_radio, volume_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_slider_set_value(volume_slider_radio, data.vol, LV_ANIM_ON);	
	lv_slider_set_range(volume_slider_filtros, 0, 100);
	
	lv_obj_t * mute_btn = lv_btn_create(panel_vol);
	lv_obj_add_event_cb(mute_btn, mute_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_play_btn = lv_label_create(mute_btn);
	lv_label_set_text_fmt(label_play_btn, "%s  Mute", LV_SYMBOL_MUTE);
	
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
	
	lv_obj_t * panel_salida = lv_obj_create(tabview);
	lv_obj_set_height(panel_salida, LV_SIZE_CONTENT);
	lv_obj_set_width(panel_salida , LV_SIZE_CONTENT);
	crear_panel_salida(panel_salida);
	
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
	lv_obj_t * tab_mp3 = lv_obj_create(tabview);
	lv_obj_set_height(tab_mp3, LV_SIZE_CONTENT);
	lv_obj_set_width(tab_mp3, 750);

	lv_obj_t * name = lv_label_create(tab_mp3);
	lv_label_set_text(name, TITLE);
	lv_obj_add_style(name, &style_title, 0);
	
	lv_obj_t * label_freq_input = lv_label_create(tab_mp3);
	lv_label_set_text_fmt(label_freq_input, "Canciones");
		
	lv_obj_t * canciones = lv_dropdown_create(tab_mp3);
	lv_dropdown_set_options_static(canciones, data.songs);
	
		
	/* Create seek up and down buttons */
	lv_obj_t * label_seek = lv_label_create(tab_mp3);
	lv_label_set_text_fmt(label_seek, "Control");
	
	lv_obj_t * play_btn = lv_btn_create(tab_mp3);
	lv_obj_add_event_cb(play_btn, seek_up_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_play_btn = lv_label_create(play_btn);
	lv_label_set_text_fmt(label_play_btn, "Play");
		
	lv_obj_t * repeat_btn = lv_btn_create(tab_mp3);
	lv_obj_add_event_cb(repeat_btn, seek_down_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_repeat_btn = lv_label_create(repeat_btn);
	lv_label_set_text_fmt(label_repeat_btn, "Bucle");
	
	lv_obj_t * prev_btn = lv_btn_create(tab_mp3);
	lv_obj_add_event_cb(prev_btn, seek_up_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_seek_up = lv_label_create(prev_btn);
	lv_label_set_text_fmt(label_seek_up, "Prev");
		
	lv_obj_t * next_btn = lv_btn_create(tab_mp3);
	lv_obj_add_event_cb(next_btn, seek_down_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_next_btn = lv_label_create(next_btn);
	lv_label_set_text_fmt(label_next_btn, "Next");

	/* Create a volume slider */
	label_volumen = lv_label_create(tab_mp3);
	lv_label_set_text_fmt(label_volumen, "Volumen");
	volume_slider_mp3 = lv_slider_create(tab_mp3);
	lv_obj_add_event_cb(volume_slider_mp3, volume_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_slider_set_value(volume_slider_mp3, data.vol, LV_ANIM_ON);	
	
	/* Create headphones and speakers buttons */
	lv_obj_t * label_salida = lv_label_create(tab_mp3);
	lv_label_set_text_fmt(label_salida, "Salida");
    
	lv_obj_t * headphones_btn = lv_btn_create(tab_mp3);
	lv_obj_add_event_cb(headphones_btn, headphones_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_headphones = lv_label_create(headphones_btn);
	lv_label_set_text_fmt(label_headphones, "Cascos");
		
    lv_obj_t * speakers_btn = lv_btn_create(tab_mp3);
    lv_obj_add_event_cb(speakers_btn, speakers_cb, LV_EVENT_CLICKED, NULL);
		lv_obj_t * label_altavoz = lv_label_create(speakers_btn);
		lv_label_set_text_fmt(label_altavoz, "Altavoz");
		
		/* Define the grid */
    static int32_t grid_col_dsc[] = {LV_GRID_FR(2), 10, LV_GRID_FR(1), LV_GRID_FR(1), 10, LV_GRID_FR(2), 10, LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
    static int32_t grid_row_dsc[] = {
			LV_GRID_CONTENT,           /* Title    */
			LV_GRID_CONTENT,  				 /* Subtitle */
			LV_GRID_CONTENT,                   
			LV_GRID_CONTENT,           /* Box      */
			LV_GRID_CONTENT,           /* Box      */
			LV_GRID_TEMPLATE_LAST
		};
		
    lv_obj_set_grid_dsc_array(tab_mp3, grid_col_dsc, grid_row_dsc);
		
		lv_obj_set_grid_cell(tab_mp3, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
		
		lv_obj_set_grid_cell(name, LV_GRID_ALIGN_START, 0, 4, LV_GRID_ALIGN_START, 0, 1);
		lv_obj_set_grid_cell(label_freq_input, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 1, 1);
		lv_obj_set_grid_cell(canciones, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
		
		lv_obj_set_grid_cell(label_seek, LV_GRID_ALIGN_STRETCH, 2, 3, LV_GRID_ALIGN_STRETCH, 1, 1);
		lv_obj_set_grid_cell(play_btn, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_START, 2, 1);
		lv_obj_set_grid_cell(repeat_btn, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_START, 2, 1);
		lv_obj_set_grid_cell(prev_btn, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_START, 3, 1);
		lv_obj_set_grid_cell(next_btn, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_START, 3, 1);
		
		lv_obj_set_grid_cell(label_volumen, LV_GRID_ALIGN_STRETCH, 5, 1, LV_GRID_ALIGN_START, 1, 1);
		lv_obj_set_grid_cell(volume_slider_mp3, LV_GRID_ALIGN_STRETCH, 5, 1, LV_GRID_ALIGN_START, 2, 1);
		
		lv_obj_set_grid_cell(label_salida, LV_GRID_ALIGN_STRETCH, 7, 1, LV_GRID_ALIGN_START, 1, 1);
		lv_obj_set_grid_cell(headphones_btn, LV_GRID_ALIGN_STRETCH, 7, 1, LV_GRID_ALIGN_START, 2, 1);
		lv_obj_set_grid_cell(speakers_btn, LV_GRID_ALIGN_STRETCH, 7, 1, LV_GRID_ALIGN_START, 3, 1);

}


static void create_filters_content(lv_obj_t * tabview){
	lv_obj_t * panel_title = lv_obj_create(tabview);	  // En este panel va el titulo y los nombres
	
	lv_obj_t * panel1 = lv_obj_create(tabview);			    // En este panel van los filtros del ecualizador
	lv_obj_set_height(panel1, LV_SIZE_CONTENT);
	lv_obj_set_width(panel1, LV_SIZE_CONTENT);
	
	lv_obj_t * panel_config = lv_obj_create(tabview);		// En este panel va lo de guardar config
	lv_obj_set_height(panel_config, LV_SIZE_CONTENT);
	lv_obj_set_width(panel_config, LV_SIZE_CONTENT);
	
	lv_obj_t * panel_vol = lv_obj_create(tabview);			     // Aqui el volumen
	lv_obj_set_height(panel_vol, LV_SIZE_CONTENT);
	lv_obj_set_width(panel_vol, LV_SIZE_CONTENT);
	
	lv_obj_t * panel_salida = lv_obj_create(tabview);			// Y aqui la salida
	lv_obj_set_height(panel_salida, LV_SIZE_CONTENT);
	lv_obj_set_width(panel_salida, LV_SIZE_CONTENT);
	
	/* Titulo */
	crear_panel_titulo(panel_title);
	
	label_coef = lv_label_create(panel_title);
	lv_label_set_text(label_coef, "Patata");
	
	/* Ecualizador */
	lv_obj_t * ecualizador_label = lv_label_create(panel1);
	lv_label_set_text(ecualizador_label, "Ecualizador");
	lv_obj_add_style(ecualizador_label, &style_subtitle, 0);
	
	/* Banda 1 */
	lv_obj_t * slider_b1 = lv_slider_create(panel1);
	lv_obj_set_size(slider_b1, 20, 150);
	lv_slider_set_value(slider_b1, 50, LV_ANIM_OFF);
	
	lv_obj_t * slider_b1_label = lv_label_create(panel1);
	lv_label_set_text(slider_b1_label, data.fcentral[0]);
	lv_obj_add_style(slider_b1_label, &style_text_muted, 0);
	/* Banda 2 */
	lv_obj_t * slider_b2 = lv_slider_create(panel1);
	lv_obj_set_size(slider_b2, 20, 150);
	lv_slider_set_value(slider_b2, 50, LV_ANIM_OFF);
	
	lv_obj_t * slider_b2_label = lv_label_create(panel1);
	lv_label_set_text(slider_b2_label, data.fcentral[1]);
	lv_obj_add_style(slider_b2_label, &style_text_muted, 0);
	/* Banda 3 */
	lv_obj_t * slider_b3 = lv_slider_create(panel1);
	lv_obj_set_size(slider_b3, 20, 150);
	lv_slider_set_value(slider_b3, 50, LV_ANIM_OFF);
	
	lv_obj_t * slider_b3_label = lv_label_create(panel1);
	lv_label_set_text(slider_b3_label, data.fcentral[2]);
	lv_obj_add_style(slider_b3_label, &style_text_muted, 0);
	/* Banda 4 */
	lv_obj_t * slider_b4 = lv_slider_create(panel1);
	lv_obj_set_size(slider_b4, 20, 150);
	lv_slider_set_value(slider_b4, 50, LV_ANIM_OFF);
	
	lv_obj_t * slider_b4_label = lv_label_create(panel1);
	lv_label_set_text(slider_b4_label, data.fcentral[3]);
	lv_obj_add_style(slider_b4_label, &style_text_muted, 0);
	/* Banda 5 */
	lv_obj_t * slider_b5 = lv_slider_create(panel1);
	lv_obj_set_size(slider_b5, 20, 150);
	lv_slider_set_value(slider_b5, 50, LV_ANIM_OFF);
	
	lv_obj_t * slider_b5_label = lv_label_create(panel1);
	lv_label_set_text(slider_b5_label, data.fcentral[4]);
	lv_obj_add_style(slider_b5_label, &style_text_muted, 0);
	
	/* Volumen */
//	crear_panel_volumen(panel3, volume_slider_filtros);
	/* Volumen */
	lv_obj_set_height(panel_vol, LV_SIZE_CONTENT);
	lv_obj_set_width(panel_vol , LV_SIZE_CONTENT);
	label_volumen = lv_label_create(panel_vol);
	lv_label_set_text_fmt(label_volumen, "Volumen");
	lv_obj_add_style(label_volumen, &style_subtitle, 0);  
	
	volume_slider_filtros = lv_slider_create(panel_vol);
	lv_obj_add_event_cb(volume_slider_filtros, volume_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_slider_set_value(volume_slider_filtros, data.vol, LV_ANIM_ON);	
//	lv_slider_set_range(volume_slider_filtros, 0, 100);
	
	lv_obj_t * mute_btn = lv_btn_create(panel_vol);
	lv_obj_add_event_cb(mute_btn, mute_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_play_btn = lv_label_create(mute_btn);
	lv_label_set_text_fmt(label_play_btn, "%s  Mute", LV_SYMBOL_MUTE);
	
	#if DRAGGABBLE
	lv_obj_add_event_cb(panel_title, drag_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(panel1, drag_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(panel_config, drag_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(panel_vol, drag_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(panel_salida, drag_event_handler, LV_EVENT_ALL, NULL);
	#endif
	
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
	
	lv_obj_set_grid_cell(label_volumen,         LV_GRID_ALIGN_CENTER,  0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(volume_slider_filtros, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
	lv_obj_set_grid_cell(mute_btn,              LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 4, 1);
	
	
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
	
	/* Para guardar configuraci�n */
	static int32_t grid_conf_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_conf_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Boton     */
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(tabview, grid_main_col_dsc, grid_main_row_dsc);
	
	/* Titulo: Real Time Audio Processor & names. En el fondo de la pagina */
	lv_obj_set_grid_cell(panel_title, LV_GRID_ALIGN_STRETCH, 0, 3, LV_GRID_ALIGN_STRETCH, 2, 1);
	lv_obj_set_grid_cell(label_coef, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
	
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
	lv_obj_set_grid_cell(save_btn, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
//	lv_obj_set_grid_cell(list_save, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	
	/* Salida */
	lv_obj_set_grid_cell(panel_salida, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 2);


//	lv_coord_t x = lv_obj_get_x(panel_title);
//	lv_coord_t y = lv_obj_get_y(panel_title);
//	lv_obj_set_layout(tabview, LV_LAYOUT_NONE);
//	lv_obj_set_pos(panel_title, x, y);
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


static void volume_cb(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * obj = lv_event_get_target(e);

	data.vol = lv_slider_get_value(obj);

	if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
		int32_t * s = lv_event_get_param(e);
		 *s = LV_MAX(*s, 60);
	}
	if(obj == volume_slider_mp3){
		lv_slider_set_value(volume_slider_radio, data.vol, LV_ANIM_ON);
		lv_slider_set_value(volume_slider_filtros, data.vol, LV_ANIM_ON);
	} else if(obj == volume_slider_radio){
		lv_slider_set_value(volume_slider_mp3, data.vol, LV_ANIM_ON);
		lv_slider_set_value(volume_slider_filtros, data.vol, LV_ANIM_ON);
	} else if(obj == volume_slider_filtros){
		lv_slider_set_value(volume_slider_mp3, data.vol, LV_ANIM_ON);
		lv_slider_set_value(volume_slider_radio, data.vol, LV_ANIM_ON);
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

static void mute_cb(lv_event_t * e) {
	static uint8_t prev_volume;
	
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * btn = lv_event_get_target(e);
	lv_obj_t * panel = lv_obj_get_parent(btn);
	
	/* Find the slider within the panel */
//    lv_obj_t * slider = NULL;
//    lv_obj_t * child = lv_obj_get_child(panel, 0);
//		int cnt = lv_obj_get_child_cnt(panel);
//    for (int i = 0; i <cnt; i++){
//        if(lv_obj_get_class(child) == lv_obj_get_class(volume_slider_filtros)) {
//            slider = child;
//            break;
//        }
//        child = lv_obj_get_child(panel, i);
//    }

//    /* If no slider was found, return */
//    if(!slider) return;
	
	if(code == LV_EVENT_CLICKED) {
		int current_volume = lv_slider_get_value(volume_slider_radio);
		
		lv_anim_t a, b, c;
		lv_anim_init(&a); lv_anim_init(&b); lv_anim_init(&c);
//		lv_anim_set_completed_cb(&a, (lv_anim_completed_cb_t) lv_event_send); lv_anim_set_completed_cb(&b, (lv_anim_completed_cb_t) lv_event_send); lv_anim_set_completed_cb(&c, (lv_anim_completed_cb_t) lv_event_send);
//		lv_anim_set_ready_cb(&a, (lv_anim_ready_cb_t) lv_event_send); lv_anim_set_ready_cb(&b, (lv_anim_ready_cb_t) lv_event_send); lv_anim_set_ready_cb(&c, (lv_anim_ready_cb_t) lv_event_send);
		lv_anim_set_var(&a, volume_slider_radio); lv_anim_set_var(&b, volume_slider_mp3); lv_anim_set_var(&c, volume_slider_filtros);
		lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_slider_set_value); lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t) lv_slider_set_value); lv_anim_set_exec_cb(&c, (lv_anim_exec_xcb_t) lv_slider_set_value);
		int duracion_ms = 250;
		lv_anim_set_time(&a, duracion_ms); lv_anim_set_time(&b, duracion_ms); lv_anim_set_time(&c, duracion_ms);
		
		if(current_volume == 0) {
			lv_anim_set_values(&a, 0, prev_volume); lv_anim_set_values(&b, 0, prev_volume); lv_anim_set_values(&c, 0, prev_volume);
			lv_anim_start(&a); lv_anim_start(&b); lv_anim_start(&c);
//			lv_slider_set_value(volume_slider_radio, prev_volume, LV_ANIM_ON);
//			lv_slider_set_value(volume_slider_mp3, prev_volume, LV_ANIM_ON);
//			lv_slider_set_value(volume_slider_filtros, prev_volume, LV_ANIM_ON);
		} else {
			prev_volume = current_volume;
			lv_anim_set_values(&a, current_volume, 0); lv_anim_set_values(&b, current_volume, 0); lv_anim_set_values(&c, current_volume, 0);
			lv_anim_start(&a); lv_anim_start(&b); lv_anim_start(&c);
//			lv_slider_set_value(volume_slider_radio, 0, LV_ANIM_ON);
//			lv_slider_set_value(volume_slider_mp3, 0, LV_ANIM_ON);
//			lv_slider_set_value(volume_slider_filtros, 0, LV_ANIM_ON);
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

LV_IMAGE_DECLARE(img_hand);

lv_obj_t * needle_line;
lv_obj_t * needle_img;

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

   lv_obj_t * label = lv_obj_get_child(var, 1);
   lv_label_set_text_fmt(label, "%"LV_PRId32, v);
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
	
	lv_scale_set_range(scale_consumo, 0, 2000);
	lv_scale_set_total_tick_count(scale_consumo,17);
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
	lv_scale_section_set_range(section, 0, 500);
	lv_scale_section_set_style(section, LV_PART_MAIN, &scale_consumo_section1_main_style);
	lv_scale_section_set_style(section, LV_PART_INDICATOR, &scale_consumo_section1_indicator_style);
	lv_scale_section_set_style(section, LV_PART_ITEMS, &scale_consumo_section1_tick_style);
	section = lv_scale_add_section(scale_consumo);
	lv_scale_section_set_range(section, 500, 1500);
	lv_scale_section_set_style(section, LV_PART_MAIN, &scale_consumo_section2_main_style);
	lv_scale_section_set_style(section, LV_PART_INDICATOR, &scale_consumo_section2_indicator_style);
	lv_scale_section_set_style(section, LV_PART_ITEMS, &scale_consumo_section2_tick_style);

	section = lv_scale_add_section(scale_consumo);
	lv_scale_section_set_range(section, 1500, 2000);
	lv_scale_section_set_style(section, LV_PART_MAIN, &scale_consumo_section3_main_style);
	lv_scale_section_set_style(section, LV_PART_INDICATOR, &scale_consumo_section3_indicator_style);
	lv_scale_section_set_style(section, LV_PART_ITEMS, &scale_consumo_section3_tick_style);

	needle_consumo = lv_line_create(scale_consumo);

	lv_obj_set_style_line_width(needle_consumo, 5, 0);
	lv_obj_set_style_line_rounded(needle_consumo, true, 0);
	lv_obj_set_style_line_color(needle_consumo, lv_palette_main(LV_PALETTE_RED), 0);
		
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
	
	/* Slider */
	slider_freq = lv_slider_create(container);
	
	lv_slider_set_range(slider_freq, 87 * 10, 108 * 10); // Solo admite enteros: Se multiplica x 10 y asi tenemos el decimal con una division
	
	static lv_style_t *style_knob;
	lv_style_init(style_knob);
	lv_style_set_bg_opa(style_knob, LV_OPA_COVER);
	lv_style_set_bg_color(style_knob, lv_palette_main(LV_PALETTE_RED));
	lv_style_set_radius(style_knob, LV_RADIUS_CIRCLE);
	lv_style_set_border_width(style_knob, 2);
	lv_style_set_pad_all(style_knob, 2);

	lv_obj_remove_style_all(slider_freq); 
	lv_obj_add_style(slider_freq, style_knob, LV_PART_KNOB);	
	
	lv_obj_add_event_cb(slider_freq, frequency_set_cb, LV_EVENT_VALUE_CHANGED, NULL);
	
	static int32_t grid_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
			LV_GRID_CONTENT,  /* Freq      */
			LV_GRID_CONTENT,  /* Scale     */
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(container, grid_col_dsc, grid_row_dsc);
	
	lv_obj_set_grid_cell(label_freq_input, LV_GRID_ALIGN_CENTER,  0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(textarea_freq,    LV_GRID_ALIGN_CENTER,  0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
	lv_obj_set_grid_cell(scale_freq,       LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
	lv_obj_set_grid_cell(slider_freq,      LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START,   2, 1);
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
static void crear_panel_salida (lv_obj_t * container){
	lv_obj_t * label_salida = lv_label_create(container);
	lv_label_set_text_fmt(label_salida, "Salida");
	lv_obj_add_style(label_salida, &style_subtitle, 0);  
	
	lv_obj_t * headphones_btn = lv_btn_create(container);
	lv_obj_add_event_cb(headphones_btn, headphones_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_headphones = lv_label_create(headphones_btn);
	lv_label_set_text_fmt(label_headphones, "Cascos");
	
	lv_obj_t * speakers_btn = lv_btn_create(container);
	lv_obj_add_event_cb(speakers_btn, speakers_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_altavoz = lv_label_create(speakers_btn);
	lv_label_set_text_fmt(label_altavoz, "Altavoz");

	static int32_t grid_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Up        */
			LV_GRID_CONTENT,  /* Down      */
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(container, grid_col_dsc, grid_row_dsc);
	
	lv_obj_set_grid_cell(label_salida,   LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(headphones_btn, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(speakers_btn,   LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 3, 1);
}
static void crear_panel_volumen (lv_obj_t * container, lv_obj_t * vol_slider){
/* No funciona. Quizas pasando un boton mute como parametro o haciendo mutes globales? */	
	
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
	lv_label_set_text_fmt(label_salida, "Camino audio");
  lv_obj_add_style(label_salida, &style_subtitle, 0);  
	
	lv_obj_t * headphones_btn = lv_btn_create(container);
	lv_obj_add_event_cb(headphones_btn, headphones_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_headphones = lv_label_create(headphones_btn);
	lv_label_set_text_fmt(label_headphones, "Cascos");
		
	lv_obj_t * speakers_btn = lv_btn_create(container);
	lv_obj_add_event_cb(speakers_btn, speakers_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_altavoz = lv_label_create(speakers_btn);
	lv_label_set_text_fmt(label_altavoz, "Altavoz");
	
	lv_obj_t * mp3_btn = lv_btn_create(container);
	lv_obj_add_event_cb(mp3_btn, mp3_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_mp3 = lv_label_create(mp3_btn);
	lv_label_set_text_fmt(label_mp3, "MP3");
		
	lv_obj_t * radio_btn = lv_btn_create(container);
	lv_obj_add_event_cb(speakers_btn, radio_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_radio = lv_label_create(radio_btn);
	lv_label_set_text_fmt(label_radio, "Radio");
	
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
	lv_obj_set_grid_cell(headphones_btn, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(speakers_btn,   LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 3, 1);
	lv_obj_set_grid_cell(mp3_btn,        LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 5, 1);
	lv_obj_set_grid_cell(radio_btn,      LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 6, 1);
}

static void freq_slider_observer_cb(lv_observer_t * observer, lv_subject_t * subject){
	lv_obj_t *slider = lv_observer_get_target_obj(observer);
	char buf[10] = "";
	sprintf(buf, "%d", lv_slider_get_value(slider));
	lv_textarea_set_text(textarea_freq, buf);
	
}

static void frequency_set_cb(lv_event_t * e) {
	lv_obj_t * obj = lv_event_get_target(e);
	char buf[10] = "";
	if (obj == slider_freq){
		float freq = ((float)lv_slider_get_value(slider_freq))/10.0;
		sprintf(buf, "%.1f", freq);
		lv_textarea_set_text(textarea_freq, buf);
	} else if(obj == textarea_freq){
		sprintf(buf, "%s", lv_textarea_get_text(textarea_freq));
		float freq = atof(buf);
		int val = (freq*10.0); 
		lv_slider_set_value(slider_freq, val, LV_ANIM_ON);
	}
}