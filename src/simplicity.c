#include "pebble.h"

Window *window;
TextLayer *text_time_layer;
TextLayer *battery_layer;
Layer *line_layer;

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100% charged";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% charged", charge_state.charge_percent);
  }
  text_layer_set_text(battery_layer, battery_text);
}

void line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);

  handle_battery(battery_state_service_peek());
}


void handle_deinit(void) {
  tick_timer_service_unsubscribe();

  //battery_state_service_unsubscribe();
  //text_layer_destroy(battery_layer);
}

#define LINES 7
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

  Layer *window_layer = window_get_root_layer(window);

  for(int i = 0 ; i < LINES ; i++) {
    TextLayer* layer = create_text_layer();
    textLayers[i] = layer;
  }

  for(int i = LINES; --i >= 0;)
    layer_add_child(window_layer, text_layer_get_layer(textLayers[i]));

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  handle_minute_tick(NULL, MINUTE_UNIT);
}

int main(void) {
  handle_init();

  app_event_loop();

  handle_deinit();
}
