#include <pebble.h>
	
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 	1
#define KEY_LATITUDE	2
#define KEY_LONGITUDE	3
	
#define PERSIST_WEATHER	1
#define PERSIST_LATLON	2
	
/// The main watchface window
static Window *s_main_window;
/// To show text on the screen
static TextLayer *s_time_layer;
static GFont s_time_font;
/// For temperature
static TextLayer *s_weather_layer;
/// For location
static TextLayer *s_latlon_layer;
/// Date
static TextLayer *s_date_layer; 

static Layer *s_battery_canvas_layer;
static int s_battery_charge_percent;
static bool s_battery_is_charging;

static void battery_ui_update(Layer *layer, GContext *context) {
	int charge_width = 144 * s_battery_charge_percent / 100;
	APP_LOG(APP_LOG_LEVEL_INFO, "Setting charge width to %d pixels", charge_width);
	
	graphics_context_set_fill_color(context, GColorBlack);
	graphics_fill_rect(context, GRect(0, 0, charge_width, 4), 0, GCornerNone);
}

static void battery_handler(BatteryChargeState new_state) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Battery charge %d, Is charging %d!", new_state.charge_percent, new_state.is_charging ? 1 : 0);
	s_battery_charge_percent = new_state.charge_percent;
	s_battery_is_charging = new_state.is_charging;
	
	// Force an update
	layer_mark_dirty(s_battery_canvas_layer);
}

// Oh right, declaration order matters in C.
// So the file's kind of written upside-down
// in terms of execution/dependencies
static void update_time() {
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	
	// make all char buffers long-lived
	static char time_buffer[] = "00:00";
	static char date_buffer[] = "Jan 01";
		
	// move to buffer
	if(clock_is_24h_style() == true) {
		strftime(time_buffer, sizeof(time_buffer), "%H:%M", tick_time);
	} else {
		strftime(time_buffer, sizeof(time_buffer), "%I:%M", tick_time);
	}
	
	strftime(date_buffer, sizeof(date_buffer), "%b %d", tick_time);
	
	// persist to UI
	text_layer_set_text(s_time_layer, time_buffer);
	text_layer_set_text(s_date_layer, date_buffer);
	
	// Refresh weather every five minutes
	if(tick_time->tm_min % 5 == 0) {	
		APP_LOG(APP_LOG_LEVEL_INFO, "Requesting updated weather");

		DictionaryIterator *iter;
  		app_message_outbox_begin(&iter);
		
		// This is where you write to the request dictionary.
		// Here's an example I found of doing that:
		//  Tuplet tuple = (symbol == NULL)
        //              ? TupletInteger(quote_key, 1)
        //              : TupletCString(quote_key, symbol);
		//	dict_write_tuplet(iter, &tuple);
		
  		dict_write_end(iter);
  		app_message_outbox_send();
	}
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

/// Called when the window is loaded
static void main_window_load(Window *window) {
	
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CODE_BOLD_60));
	
	// For coordinates, the Pebble screen is 144x168,
	// horizontal x vertical.
	// GRect(topLeft, topRight, width, height)
	
	// 4 	<----- Battery fill ----->
	// 28	Date (^ 4 top padding)
	// 60	Time (some top padding)
	// 32	Weather
	// 32	LatLon
	// 9	|
	
	// Create date Layer
	s_date_layer = text_layer_create(GRect(0, 4, 144, 28));
	text_layer_set_background_color(s_date_layer, GColorClear);
	text_layer_set_text_color(s_date_layer, GColorBlack);
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
	text_layer_set_text(s_date_layer, "");
	text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
	
	// Be sure to _destroy this on _unload()
	s_time_layer = text_layer_create(GRect(0, 28, 144, 60));
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorBlack);
	text_layer_set_font(s_time_layer, s_time_font);
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

	// Create temperature Layer
	s_weather_layer = text_layer_create(GRect(0, 95, 144, 32));
	text_layer_set_background_color(s_weather_layer, GColorBlack);
	text_layer_set_text_color(s_weather_layer, GColorWhite);
	text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
	text_layer_set_text(s_weather_layer, "");
	text_layer_set_font(s_weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
	
	// Create location Layer
	s_latlon_layer = text_layer_create(GRect(0, 127, 144, 32));
	text_layer_set_background_color(s_latlon_layer, GColorClear);
	text_layer_set_text_color(s_latlon_layer, GColorBlack);
	text_layer_set_text_alignment(s_latlon_layer, GTextAlignmentCenter);
	text_layer_set_text(s_latlon_layer, "");
	text_layer_set_font(s_latlon_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_latlon_layer));
	
	s_battery_canvas_layer = layer_create(GRect(0, 0, 144, 4));
	layer_add_child(window_get_root_layer(window), s_battery_canvas_layer);
	layer_set_update_proc(s_battery_canvas_layer, battery_ui_update);
}

/// Called when the window is unloaded
static void main_window_unload(Window *window) {
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_weather_layer);
	text_layer_destroy(s_latlon_layer);
	text_layer_destroy(s_date_layer);
	layer_destroy(s_battery_canvas_layer);
	fonts_unload_custom_font(s_time_font);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	
	static char temp_buffer[8];
	static char cond_buffer[32];
	static char weather_buffer[32];
	
	static char lat_buffer[8];
	static char lon_buffer[8];
	static char latlon_buffer[32];
	
	APP_LOG(APP_LOG_LEVEL_INFO, "response received from phone");
	
	// This is how to pull a key out on demand
	/*
		Tuple *thing = dict_find(iterator, KEY_TEMPERATURE);
		if(thing == NULL) {
			APP_LOG(APP_LOG_LEVEL_ERROR, "error finding temperature item");

		} else {
			APP_LOG(APP_LOG_LEVEL_WARNING, "found it!");
		}
	*/
	
	// This is how to iteratively find all key/value pairs
	Tuple *t = dict_read_first(iterator);
	while(t != NULL) {
		switch(t->key) {
			case KEY_TEMPERATURE:
				snprintf(temp_buffer, sizeof(temp_buffer), "%d°F", (int)t->value->int32);
				break;
			
			case KEY_CONDITIONS:
				snprintf(cond_buffer, sizeof(cond_buffer), "%s", t->value->cstring);
				break;
			
			case KEY_LATITUDE:
				snprintf(lat_buffer, sizeof(lat_buffer), "%.6s", t->value->cstring);
				break;
			
			case KEY_LONGITUDE:
				snprintf(lon_buffer, sizeof(lon_buffer), t->value->cstring[0] == '-' ? "%.7s" : "%.6s", t->value->cstring);
				break;
			
			default:
				APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
				break;
		}
		
		t = dict_read_next(iterator);
	}
	
	// Assemble
	snprintf(weather_buffer, sizeof(weather_buffer), "%s, %s", temp_buffer, cond_buffer);
	text_layer_set_text(s_weather_layer, weather_buffer);
	
	//snprintf(latlon_buffer, sizeof(latlon_buffer), "%s, %s", lat_buffer, lon_buffer);
	snprintf(latlon_buffer, sizeof(latlon_buffer), "%s, %s", "41.879", "-87.636");
	text_layer_set_text(s_latlon_layer, latlon_buffer);
	
	persist_write_string(PERSIST_WEATHER, weather_buffer);
	persist_write_string(PERSIST_LATLON, latlon_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed because %d", reason);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send succeeded");
}


static void init() {
	// SDK best practice: make sure _create() methods
	// in init() are always paired with _destroy()
	// methods in deinit()
	s_main_window = window_create();
	
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	
	window_stack_push(s_main_window, true);
	
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	update_time();
	
	battery_state_service_subscribe(battery_handler);
	battery_handler(battery_state_service_peek());
	
	// SDK best practice: register callbacks before opening appMessage
	app_message_register_inbox_received(inbox_received_callback);	
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
	
	// Now open appMessage
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
	if(persist_exists(PERSIST_WEATHER)) {
		static char weather_buffer[32];
		persist_read_string(PERSIST_WEATHER, weather_buffer, sizeof(weather_buffer));
		text_layer_set_text(s_weather_layer, weather_buffer);
	}
	if(persist_exists(PERSIST_LATLON)) {
		static char latlon_buffer[32];
		persist_read_string(PERSIST_LATLON, latlon_buffer, sizeof(latlon_buffer));
		text_layer_set_text(s_latlon_layer, latlon_buffer);
	}
}

static void deinit() {
	window_destroy(s_main_window);
}

/// Pretty standard service wireup
int main(void) {
	init();
	app_event_loop();
	deinit();
}