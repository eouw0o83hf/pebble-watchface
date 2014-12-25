#include <pebble.h>
	
/// The main watchface window
static Window *s_main_window;
	
/// Called when the window is loaded
static void main_window_load(Window *window) {
	
}

/// Called when the window is unloaded
static void main_window_unload(Window *window) {
	
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