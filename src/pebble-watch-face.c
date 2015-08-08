#include <pebble.h>
#include <time.h>

static Window *s_main_window;

static TextLayer *s_time_layer;
static Layer *s_canvas_layer;

static void update_time() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Create a long-lived buffer
    static char buffer[] = "00:00";

    // Write the current hours and minutes into the buffer
    if(clock_is_24h_style() == true) {
        // Use 24 hour format
        strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
    } else {
        // Use 12 hour format
        strftime(buffer, sizeof("00:00"), "%I:%M %p", tick_time);
    }

    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, buffer);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    double hour;
    double thour;
    double minute;
    double seconds;

    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 8);

    // Set up 24h bar.
    hour = tick_time->tm_hour;
    hour = hour * ((double)144 / (double)24);
    graphics_context_set_stroke_color(ctx, GColorOrange);
    graphics_draw_line(ctx, GPoint(0, 4), GPoint(hour, 4));

    // Set up 12h bar.
    thour = tick_time->tm_hour;
    if (thour > 11) {
        thour = thour - 12;
    }
    thour = thour * ((double)144 / (double)11);
    graphics_context_set_stroke_color(ctx, GColorGreen);
    graphics_draw_line(ctx, GPoint(0, 16), GPoint(thour, 16));

    // Set up minute bar.
    minute = tick_time->tm_min * ((double)144 / (double)59);
    graphics_context_set_stroke_color(ctx, GColorPictonBlue);
    graphics_draw_line(ctx, GPoint(0, 28), GPoint(minute, 28));

    // Set up seconds bar.
    seconds = tick_time->tm_sec * ((double)144 / (double)59);
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_line(ctx, GPoint(0, 40), GPoint(seconds, 40));
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
    layer_set_update_proc(s_canvas_layer, canvas_update_proc);
}

static void main_window_load(Window *window) {
    // Create time TextLayer
    s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorWhite);

    // Improve the layout to be more like a watchface
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

    // Add it as a child layer to the Window's root layer
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

    s_canvas_layer = layer_create(GRect(0, 0, 244, 48));
    layer_add_child(window_get_root_layer(window), s_canvas_layer);
}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
    // Destroy bars layer
    layer_destroy(s_canvas_layer);
}

static void init() {
    // Create main Window element and assign to pointer
    s_main_window = window_create();
    window_set_background_color(s_main_window, GColorOxfordBlue);

    // Set handlers to manage the elements inside the Window
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    // Show the Window on the watch, with animated=true
    window_stack_push(s_main_window, true);

    // Register with TickTimerService
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

    // Make sure the time is displayed from the start
    update_time();
}

static void deinit() {
    // Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
