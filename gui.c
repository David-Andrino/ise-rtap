#include "gui.h"

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

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void home_create(lv_obj_t * parent);
static void create_radio_content(lv_obj_t * tabview);
static void create_mp3_content(lv_obj_t * tabview);
static void create_filters_content(lv_obj_t * tabview);

static void ta_event_cb(lv_event_t * e);
static void slider_event_cb(lv_event_t * e);
static void tabview_delete_event_cb(lv_event_t * e);

static void drag_event_handler(lv_event_t * e);
static void frequency_set_cb(lv_event_t * e) {}
static void seek_up_cb(lv_event_t * e) {}
static void seek_down_cb(lv_event_t * e) {}
static void volume_cb(lv_event_t * e);
static void headphones_cb(lv_event_t * e) {}
static void speakers_cb(lv_event_t * e) {}
static void mute_cb(lv_event_t * e);
static void save_cb(lv_event_t * e){}
/**********************
 *  STATIC VARIABLES
 **********************/
static disp_size_t disp_size;

static lv_obj_t * tv;
static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_icon;
static lv_style_t style_bullet;

static const lv_font_t * font_large;
static const lv_font_t * font_normal;

static lv_obj_t * label_volumen;
static lv_obj_t *volume_slider_radio, *volume_slider_mp3, *volume_slider_filtros;		// Necesario que sea global para poder copiar el valor. No puede haber solo un objeto compartido porque falla (Necesario uno por tab).

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
	font_large = LV_FONT_DEFAULT;
	font_normal = LV_FONT_DEFAULT;
	int32_t tab_h;
	tab_h = 70;
	
#if LV_FONT_MONTSERRAT_24
		font_large     = &lv_font_montserrat_24;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_24 is not enabled. Using LV_FONT_DEFAULT instead.");
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
//color_changer_create(tv);
}


static void home_create(lv_obj_t * parent){
	lv_obj_t * panel1 = lv_obj_create(parent);
	lv_obj_set_height(panel1, LV_SIZE_CONTENT);

//    LV_IMAGE_DECLARE(img_demo_widgets_avatar);
//    lv_obj_t * avatar = lv_image_create(panel1);
//    lv_image_set_src(avatar, &img_demo_widgets_avatar);
//    lv_image_set_src(avatar, "A:lvgl/demos/widgets/assets/avatar.png")

	lv_obj_t * name = lv_label_create(panel1);
	lv_label_set_text(name, TITLE);
	lv_obj_add_style(name, &style_title, 0);

	lv_obj_t * dsc = lv_label_create(panel1);
	lv_obj_add_style(dsc, &style_text_muted, 0);
	lv_label_set_text(dsc, "No se que poner pero queda bonito (Nombres ?)");
	lv_label_set_long_mode(dsc, LV_LABEL_LONG_WRAP);

	lv_obj_t * email_icn = lv_label_create(panel1);
	lv_obj_add_style(email_icn, &style_icon, 0);
	lv_label_set_text(email_icn, LV_SYMBOL_ENVELOPE);

	lv_obj_t * email_label = lv_label_create(panel1);
	lv_label_set_text(email_label, "david feo");

	lv_obj_t * call_icn = lv_label_create(panel1);
	lv_obj_add_style(call_icn, &style_icon, 0);
	lv_label_set_text(call_icn, LV_SYMBOL_CALL);

	lv_obj_t * call_label = lv_label_create(panel1);
	lv_label_set_text(call_label, "o los nombres aqui? ");

	lv_obj_t * invite_btn = lv_button_create(panel1);
	lv_obj_add_state(invite_btn, LV_STATE_DISABLED);
	lv_obj_set_height(invite_btn, LV_SIZE_CONTENT);

	/*Create a keyboard*/
	lv_obj_t * kb = lv_keyboard_create(lv_screen_active());
	lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

	/*Create the second panel*/
	lv_obj_t * panel2 = lv_obj_create(parent);
	lv_obj_set_height(panel2, LV_SIZE_CONTENT);

	lv_obj_t * panel2_title = lv_label_create(panel2);
	lv_label_set_text(panel2_title, "Elige camino de audio: ");
	lv_obj_add_style(panel2_title, &style_title, 0);

	lv_obj_t * salida_audio_label = lv_label_create(panel2);
	lv_label_set_text(salida_audio_label, "MP3 o Cascos");
	lv_obj_add_style(salida_audio_label, &style_text_muted, 0);

	lv_obj_t * user_name = lv_textarea_create(panel2);
	lv_textarea_set_one_line(user_name, true);
	lv_textarea_set_placeholder_text(user_name, "Esto deberia ser un boton");
	lv_obj_add_event_cb(user_name, ta_event_cb, LV_EVENT_ALL, kb);

	lv_obj_t * audio_source_label = lv_label_create(panel2);
	lv_label_set_text(audio_source_label, "Aqui va lo de altavoz o cascos");
	lv_obj_add_style(audio_source_label, &style_text_muted, 0);

	lv_obj_t * gender = lv_dropdown_create(panel2);
	lv_dropdown_set_options_static(gender, "Deberia\nser\nun\nboton");

 
	/*Create the third panel*/
	lv_obj_t * panel3 = lv_obj_create(parent);
	lv_obj_t * panel3_title = lv_label_create(panel3);
	lv_label_set_text(panel3_title, "Consumo");
	lv_obj_add_style(panel3_title, &style_title, 0);

	lv_obj_t * consumo_label = lv_label_create(panel3);
	lv_label_set_text(consumo_label, "Aqui va un widget");
	lv_obj_add_style(consumo_label, &style_text_muted, 0);

	lv_obj_t * slider1 = lv_slider_create(panel3);
	lv_obj_set_width(slider1, LV_PCT(95));
	lv_obj_add_event_cb(slider1, slider_event_cb, LV_EVENT_ALL, NULL);
	lv_obj_add_flag(slider1, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
	lv_obj_refresh_ext_draw_size(slider1);

	lv_obj_t * bajo_consumo_label = lv_label_create(panel3);
	lv_label_set_text(bajo_consumo_label, "Bajo consumo");
	lv_obj_add_style(bajo_consumo_label, &style_text_muted, 0);

	lv_obj_t * sw1 = lv_switch_create(panel3);
	
	
	static int32_t grid_main_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_main_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

	/*Create the top panel*/
	static int32_t grid_1_col_dsc[] = {LV_GRID_CONTENT, 5, LV_GRID_CONTENT, LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_1_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, 10, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

	static int32_t grid_2_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_2_row_dsc[] = {
			LV_GRID_CONTENT,  /*Title*/
			5,                /*Separator*/
			LV_GRID_CONTENT,  /*Box title*/
			30,               /*Boxes*/
			5,                /*Separator*/
			LV_GRID_CONTENT,  /*Box title*/
			30,               /*Boxes*/
			LV_GRID_TEMPLATE_LAST
	};

	lv_obj_set_grid_dsc_array(parent, grid_main_col_dsc, grid_main_row_dsc);

	lv_obj_set_grid_cell(panel1, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);

	lv_obj_set_grid_dsc_array(panel1, grid_1_col_dsc, grid_1_row_dsc);
//        lv_obj_set_grid_cell(avatar, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 5);
	lv_obj_set_grid_cell(name, LV_GRID_ALIGN_START, 2, 2, LV_GRID_ALIGN_CENTER, 0, 1);
	lv_obj_set_grid_cell(dsc, LV_GRID_ALIGN_STRETCH, 2, 4, LV_GRID_ALIGN_START, 1, 1);
	lv_obj_set_grid_cell(email_icn, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 3, 1);
	lv_obj_set_grid_cell(email_label, LV_GRID_ALIGN_START, 3, 1, LV_GRID_ALIGN_CENTER, 3, 1);
	lv_obj_set_grid_cell(call_icn, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 4, 1);
	lv_obj_set_grid_cell(call_label, LV_GRID_ALIGN_START, 3, 1, LV_GRID_ALIGN_CENTER, 4, 1);
	
	lv_obj_set_grid_cell(invite_btn, LV_GRID_ALIGN_STRETCH, 5, 1, LV_GRID_ALIGN_CENTER, 3, 2);

	lv_obj_set_grid_cell(panel2, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 1, 1);
	lv_obj_set_grid_dsc_array(panel2, grid_2_col_dsc, grid_2_row_dsc);
	lv_obj_set_grid_cell(panel2_title, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
	lv_obj_set_grid_cell(user_name, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);
	lv_obj_set_grid_cell(salida_audio_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(gender, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 6, 1);
	lv_obj_set_grid_cell(audio_source_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 5, 1);

	lv_obj_set_grid_cell(panel3, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
	lv_obj_set_grid_dsc_array(panel3, grid_2_col_dsc, grid_2_row_dsc);
	lv_obj_set_grid_cell(panel3_title, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
	lv_obj_set_grid_cell(slider1, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 3, 1);
	lv_obj_set_grid_cell(consumo_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(bajo_consumo_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 5, 1);
}

void create_radio_content(lv_obj_t * tabview) {
    lv_obj_t * tab_radio = lv_obj_create(tabview);
		lv_obj_set_height(tab_radio, LV_SIZE_CONTENT);
		lv_obj_set_width(tab_radio, 750);
	
		lv_obj_t * name = lv_label_create(tab_radio);
		lv_label_set_text(name, TITLE);
		lv_obj_add_style(name, &style_title, 0);
	
    /* Create a frequency input */
    lv_obj_t * frequency_input = lv_textarea_create(tab_radio);
    lv_obj_set_width(frequency_input, LV_PCT(100));
    lv_textarea_set_one_line(frequency_input, true);
    lv_textarea_set_placeholder_text(frequency_input, "Introducir frecuencia [MHz]");
//    lv_obj_add_event_cb(frequency_input, frequency_set_cb, LV_EVENT_VALUE_CHANGED, NULL);
		lv_obj_add_event_cb(frequency_input, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
		lv_obj_t * label_freq_input = lv_label_create(tab_radio);
//		lv_obj_add_style(label_freq_input, &style, 0);
		lv_label_set_text_fmt(label_freq_input, "Sintonizar una frecuencia [MHz]");
		
    /* Create seek up and down buttons */
		lv_obj_t * label_seek = lv_label_create(tab_radio);
//		lv_obj_add_style(label_seek, &style_title, 0);
		lv_label_set_text_fmt(label_seek, "Buscar emisora");
		
    lv_obj_t * seek_up_btn = lv_btn_create(tab_radio);
    lv_obj_add_event_cb(seek_up_btn, seek_up_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * label_seek_up = lv_label_create(seek_up_btn);
		lv_label_set_text_fmt(label_seek_up, "Up");
		
		lv_obj_t * seek_down_btn = lv_btn_create(tab_radio);
    lv_obj_add_event_cb(seek_down_btn, seek_down_cb, LV_EVENT_CLICKED, NULL);
		lv_obj_t * label_seek_down = lv_label_create(seek_down_btn);
		lv_label_set_text_fmt(label_seek_down, "Down");
		
    /* Create a volume slider */
		label_volumen = lv_label_create(tab_radio);
		lv_label_set_text_fmt(label_volumen, "Volumen");
    volume_slider_radio = lv_slider_create(tab_radio);
    lv_obj_add_event_cb(volume_slider_radio, volume_cb, LV_EVENT_VALUE_CHANGED, NULL);
		lv_slider_set_value(volume_slider_radio, data.vol, LV_ANIM_OFF);
		
    /* Create headphones and speakers buttons */
		lv_obj_t * label_salida = lv_label_create(tab_radio);
		lv_label_set_text_fmt(label_salida, "Salida");
    
		lv_obj_t * headphones_btn = lv_btn_create(tab_radio);
    lv_obj_add_event_cb(headphones_btn, headphones_cb, LV_EVENT_CLICKED, NULL);
		lv_obj_t * label_headphones = lv_label_create(headphones_btn);
		lv_label_set_text_fmt(label_headphones, "Cascos");
		
    lv_obj_t * speakers_btn = lv_btn_create(tab_radio);
    lv_obj_add_event_cb(speakers_btn, speakers_cb, LV_EVENT_CLICKED, NULL);
		lv_obj_t * label_altavoz = lv_label_create(speakers_btn);
		lv_label_set_text_fmt(label_altavoz, "Altavoz");
		
		/* Define the grid */
    static int32_t grid_col_dsc[] = {LV_GRID_FR(1), 10, LV_GRID_FR(1), 10, LV_GRID_FR(1), 10, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t grid_row_dsc[] = {
			LV_GRID_CONTENT,           /* Title    */
			LV_GRID_CONTENT,  				 /* Subtitle */
			LV_GRID_CONTENT,                   
			LV_GRID_CONTENT,           /* Box      */
			LV_GRID_CONTENT,           /* Box      */
			LV_GRID_TEMPLATE_LAST
		};
		
    lv_obj_set_grid_dsc_array(tab_radio, grid_col_dsc, grid_row_dsc);
		
		lv_obj_set_grid_cell(tab_radio, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
		
		lv_obj_set_grid_cell(name, LV_GRID_ALIGN_START, 0, 4, LV_GRID_ALIGN_START, 0, 1);
		lv_obj_set_grid_cell(label_freq_input, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 1, 1);
		lv_obj_set_grid_cell(frequency_input, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
		
		lv_obj_set_grid_cell(label_seek, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
		lv_obj_set_grid_cell(seek_up_btn, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_START, 2, 1);
		lv_obj_set_grid_cell(seek_down_btn, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_START, 3, 1);
		
		lv_obj_set_grid_cell(label_volumen, LV_GRID_ALIGN_STRETCH, 4, 1, LV_GRID_ALIGN_START, 1, 1);
		lv_obj_set_grid_cell(volume_slider_radio, LV_GRID_ALIGN_STRETCH, 4, 1, LV_GRID_ALIGN_START, 2, 1);
		
		lv_obj_set_grid_cell(label_salida, LV_GRID_ALIGN_STRETCH, 6, 1, LV_GRID_ALIGN_START, 1, 1);
		lv_obj_set_grid_cell(headphones_btn, LV_GRID_ALIGN_STRETCH, 6, 1, LV_GRID_ALIGN_START, 2, 1);
		lv_obj_set_grid_cell(speakers_btn, LV_GRID_ALIGN_STRETCH, 6, 1, LV_GRID_ALIGN_START, 3, 1);
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
	
	lv_obj_t * panel3 = lv_obj_create(tabview);			     // Aqui el volumen
	lv_obj_set_height(panel3, LV_SIZE_CONTENT);
	lv_obj_set_width(panel3, LV_SIZE_CONTENT);
	
	lv_obj_t * panel_salida = lv_obj_create(tabview);			// Y aqui la salida
	lv_obj_set_height(panel_salida, LV_SIZE_CONTENT);
	lv_obj_set_width(panel_salida, LV_SIZE_CONTENT);
	
	#if DRAGGABBLE
	lv_obj_add_event_cb(panel_title, drag_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(panel1, drag_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(panel_config, drag_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(panel3, drag_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(panel_salida, drag_event_handler, LV_EVENT_ALL, NULL);
	#endif
	
	/* Titulo */
	lv_obj_t * name = lv_label_create(panel_title);
	lv_label_set_text(name, TITLE);
	lv_obj_add_style(name, &style_title, 0);
	
	lv_obj_t * about = lv_label_create(panel_title);
	lv_label_set_text(about, ABOUT);
	
	/* Ecualizador */
	lv_obj_t * ecualizador_label = lv_label_create(panel1);
	lv_label_set_text(ecualizador_label, "Ecualizador");
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
	label_volumen = lv_label_create(panel3);
	lv_label_set_text_fmt(label_volumen, "Volumen");
	volume_slider_filtros = lv_slider_create(panel3);
	lv_obj_add_event_cb(volume_slider_filtros, volume_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_slider_set_value(volume_slider_filtros, data.vol, LV_ANIM_ON);	
//	lv_slider_set_range(volume_slider_filtros, 0, 100);
	
	lv_obj_t * mute_btn = lv_btn_create(panel3);
	lv_obj_add_event_cb(mute_btn, mute_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_play_btn = lv_label_create(mute_btn);
	lv_label_set_text_fmt(label_play_btn, "%s  Mute", LV_SYMBOL_MUTE);
	
	/* Guardar configuracion */
//	lv_obj_t * list_save = lv_list_create(panel_config);
//	lv_list_add_text(list_save, "Guardar config");
	
	lv_obj_t * label_config = lv_label_create(panel_config);
	lv_label_set_text_fmt(label_config, "Guardar config");
	
//	lv_obj_t * save_btn = lv_list_add_btn(list_save, LV_SYMBOL_SAVE, "Save");//lv_btn_create(panel_config);
	lv_obj_t * save_btn = lv_btn_create(panel_config);
	lv_obj_add_event_cb(save_btn, save_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_save_config_btn = lv_label_create(save_btn);
	lv_label_set_text_fmt(label_save_config_btn, "%s  Save", LV_SYMBOL_SAVE);
	
	/* Botones salida de audio */
	lv_obj_t * label_salida = lv_label_create(panel_salida);
	lv_label_set_text_fmt(label_salida, "Salida");
    
	lv_obj_t * headphones_btn = lv_btn_create(panel_salida);
	lv_obj_add_event_cb(headphones_btn, headphones_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_headphones = lv_label_create(headphones_btn);
	lv_label_set_text_fmt(label_headphones, "Cascos");
		
	lv_obj_t * speakers_btn = lv_btn_create(panel_salida);
	lv_obj_add_event_cb(speakers_btn, speakers_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t * label_altavoz = lv_label_create(speakers_btn);
	lv_label_set_text_fmt(label_altavoz, "Altavoz");
	
	
	/* Para la tab */
	static int32_t grid_main_col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_main_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	
	/* Para el titulo */
	static int32_t grid_1_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_1_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

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
	
	/* Para el volumen */
	static int32_t grid_vol_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_vol_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Slider    */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Mute      */
			LV_GRID_TEMPLATE_LAST
	};
	
	/* Para guardar configuración */
	static int32_t grid_conf_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_conf_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Boton     */
			LV_GRID_TEMPLATE_LAST
	};
	
	/* Para la salida de audio */
	static int32_t grid_sal_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static int32_t grid_sal_row_dsc[] = {
			LV_GRID_CONTENT,  /* Subtitulo */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Boton     */
			5,                /* Separador */
			LV_GRID_CONTENT,  /* Boton     */
			LV_GRID_TEMPLATE_LAST
	};
	
	lv_obj_set_grid_dsc_array(tabview, grid_main_col_dsc, grid_main_row_dsc);
	
	/* Titulo: Real Time Audio Processor & names. En el fondo de la pagina */
	lv_obj_set_grid_cell(panel_title, LV_GRID_ALIGN_STRETCH, 0, 3, LV_GRID_ALIGN_STRETCH, 2, 1);
	lv_obj_set_grid_dsc_array(panel_title, grid_1_col_dsc, grid_1_row_dsc);
	
	lv_obj_set_grid_cell(name, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(about,LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
	
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
	lv_obj_set_grid_cell(panel3, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_dsc_array(panel3, grid_vol_col_dsc, grid_vol_row_dsc);
	
	lv_obj_set_grid_cell(label_volumen, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(volume_slider_filtros, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
	lv_obj_set_grid_cell(mute_btn, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_STRETCH, 4, 1);
	
	/* Guardar config */
	lv_obj_set_grid_cell(panel_config, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
	lv_obj_set_grid_dsc_array(panel_config, grid_conf_col_dsc, grid_conf_row_dsc);
	
	lv_obj_set_grid_cell(label_config, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(save_btn, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
//	lv_obj_set_grid_cell(list_save, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	
	/* Salida */
	lv_obj_set_grid_cell(panel_salida, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
	lv_obj_set_grid_dsc_array(panel_salida, grid_sal_col_dsc, grid_sal_row_dsc);
	
	lv_obj_set_grid_cell(label_salida, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
	lv_obj_set_grid_cell(headphones_btn, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
	lv_obj_set_grid_cell(speakers_btn, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 4, 1);
	
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

static void slider_event_cb(lv_event_t * e)
{
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
static void ta_event_cb(lv_event_t * e)
{
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

static void tabview_delete_event_cb(lv_event_t * e)
{
   lv_event_code_t code = lv_event_get_code(e);

   if(code == LV_EVENT_DELETE) {
       lv_style_reset(&style_text_muted);
       lv_style_reset(&style_title);
       lv_style_reset(&style_icon);
       lv_style_reset(&style_bullet);
   }
}
