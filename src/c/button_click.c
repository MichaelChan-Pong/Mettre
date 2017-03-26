#include <pebble.h>
#define MAX_TEMPO 240
#define MIN_TEMPO 1

static Window *window;
static TextLayer *text_layer;
static int tempo = 120;
static bool active = false;
static AppTimer * update_met;

static const uint32_t const segments[] = { 100 };
static VibePattern pat = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
};

int float_time_ms() {
	time_t seconds;
	uint16_t milliseconds;
	time_ms(&seconds, &milliseconds);
	return seconds * 1000 + (milliseconds);
}

void vibrate(){
  double start = float_time_ms();
  int interval = (double)(60.0 / tempo) * 1000;
  vibes_enqueue_custom_pattern(pat);
  if(active){
    double time = float_time_ms() - start;
    update_met = app_timer_register(interval - time, vibrate, NULL); 
  } else {
    app_timer_cancel(update_met);
  }
}

char *itoa10 (int value, char *result){
    char const digit[] = "0123456789";
    char *p = result;
    if (value < 0) {
        *p++ = '-';
        value *= -1;
    }
    /* move number of required chars and null terminate */
    int shift = value;
    do {
        ++p;
        shift /= 10;
    } while (shift);
    *p = '\0';

    /* populate result in reverse order */
    do {
        *--p = digit [value % 10];
        value /= 10;
    } while (value);

    return result;
}

static void set_text(){
  char *buf = malloc(sizeof(char));    /* <-- implicit NUL-terminator at the end here */
  buf = itoa10(tempo, buf);
  active ? text_layer_set_text(text_layer, buf) : text_layer_set_text(text_layer, "Paused");
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  active ^= true; //toggle metronome
  set_text(); 
  vibrate();
}
  
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(active && tempo < MAX_TEMPO){
    tempo++;
    set_text();
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(active && tempo > MIN_TEMPO){
    tempo--;
    set_text();
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create(GRect(0, 66, bounds.size.w, 40));
  text_layer_set_text(text_layer, "Start >");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}