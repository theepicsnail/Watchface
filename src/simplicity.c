#include "pebble.h"
#define WIDTH 144
#define HEIGHT 168

Window *window;
Layer *window_layer;
TextLayer *text_time_layer;
Layer *line_layer;



// --- battery ---
static char battery_text[] = "100% charged";
TextLayer *battery_layer;
static void update_battery(BatteryChargeState charge_state) {

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% charged", charge_state.charge_percent);
  }
  text_layer_set_text(battery_layer, battery_text);
}

void battery_create() {
  TextLayer* layer = text_layer_create(GRect(0, HEIGHT-24, WIDTH, 24));
  text_layer_set_text_color(layer, GColorBlack);
  text_layer_set_text_alignment(layer, GTextAlignmentLeft);
  text_layer_set_background_color(layer, GColorWhite);
  text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text(layer, "Loading");

  battery_layer = layer;

  battery_state_service_subscribe(update_battery);
  update_battery(battery_state_service_peek());

  layer_add_child(window_layer, text_layer_get_layer(layer));
}

void battery_destroy() {
  battery_state_service_unsubscribe();
}
// --- end battery ---

// --- time ---
static char time_text[] = "23:59";
TextLayer *time_layer;
static void update_time(struct tm *tick_time, TimeUnits units_changed) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Time flies!");
  strftime(time_text, sizeof(time_text), "%R", tick_time);
  text_layer_set_text(time_layer, time_text);

}
void time_create() {
  time_layer = text_layer_create(GRect(0,-13,WIDTH, 51));// ends at 48
  text_layer_set_text_color(time_layer, GColorBlack);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_background_color(time_layer, GColorWhite);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  time_t now = time(NULL);
  update_time(localtime(&now), MINUTE_UNIT);
  tick_timer_service_subscribe(MINUTE_UNIT, update_time);
}
void time_destroy() {
  tick_timer_service_unsubscribe();
}
// --- end time ---


void handle_deinit(void) {
  tick_timer_service_unsubscribe();
  time_destroy();
  battery_destroy();
  //text_layer_destroy(battery_layer);
}

#define LINES 0
const char *lines[] = {
  "%m/%d/%Y",
  "%B - [%m]",
  "%A - [%d]",
  "",
  "%R",
  "",
  ""
};

TextLayer *textLayers[LINES];
char buffer[LINES][20];
TextLayer *text_date_layer;
TextLayer *month_layer;

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {

  if (!tick_time) {
    time_t now = time(NULL);
    tick_time = localtime(&now);
  }

  for(int i = 0 ; i < LINES ; i++) {
    strftime(buffer[i], sizeof(buffer[i]), lines[i], tick_time);
    text_layer_set_text(textLayers[i], buffer[i]);
  }

}

int layer_Y = 0;
int layer_H = 24;
TextLayer* create_text_layer(void) {
  TextLayer* layer = text_layer_create(GRect(0, layer_Y, 144, layer_H + 5)); layer_Y += layer_H;
  text_layer_set_text_color(layer, GColorWhite);
  text_layer_set_text_alignment(layer, GTextAlignmentCenter);
  text_layer_set_background_color(layer, GColorBlack);
  text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));//FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text(layer, "Loading");

  return layer;
}

void handle_init(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);
  window_layer = window_get_root_layer(window);


  for(int i = 0 ; i < LINES ; i++) {
    TextLayer* layer = create_text_layer();
    textLayers[i] = layer;
  }

  for(int i = LINES; --i >= 0;)
    layer_add_child(window_layer, text_layer_get_layer(textLayers[i]));

  time_create();
  battery_create();


  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  handle_minute_tick(NULL, MINUTE_UNIT);
}

int main(void) {
  handle_init();

  app_event_loop();

  handle_deinit();
}
