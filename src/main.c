#include <pebble.h>
	
/// The main watchface window
static Window *s_main_window;
/// To show text on the screen
static TextLayer *s_time_layer;

/// Called when the window is loaded
static void main_window_load(Window *window) {
	// Be sure to _destroy this on _unload()
	s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
	
	// Text/UI initialization
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorBlack);
	text_layer_set_text(s_time_layer, "00:00");
	
	// Make it pretty
	text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
}

/// Called when the window is unloaded
static void main_window_unload(Window *window) {
	text_layer_destroy(s_time_layer);
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