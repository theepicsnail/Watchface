#include "pebble.h"
#define WIDTH 144
#define HEIGHT 168

Window *window;
Layer *window_layer;

// --- battery ---
Layer *battery_bar;
BatteryChargeState battery_state;
static void update_battery_charge(BatteryChargeState charge_state) {
  battery_state = charge_state;
  layer_mark_dirty(battery_bar);
}

void update_battery_tick(struct tm *tick_time, TimeUnits units_changed) {
  if (battery_state.is_charging)
  {
    battery_state.charge_percent = tick_time->tm_sec%11*10;
    layer_mark_dirty(battery_bar);
  }
}

void update_battery_draw(struct Layer *layer, GContext *ctx)
{
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0,0,WIDTH,HEIGHT), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(
    2 + 14*battery_state.charge_percent/10, // 2 to 142 (2px border)
    0,WIDTH,2), 0, GCornerNone);
}

void battery_create() {
  battery_bar = layer_create(GRect(0,HEIGHT-2, WIDTH, 2));
  layer_set_update_proc(battery_bar, update_battery_draw);


  battery_state_service_subscribe(update_battery_charge);
  update_battery_charge(battery_state_service_peek());

  layer_add_child(window_layer, battery_bar);
}
// --- end battery ---

// --- shortdate ---
static char shortdate_text[] = "00/00/00";
TextLayer *shortdate_layer;

static void update_shortdate(struct tm *tick_time, TimeUnits units_changed) {
  strftime(shortdate_text, sizeof(shortdate_text), "%D", tick_time);
  text_layer_set_text(shortdate_layer, shortdate_text);
}

void shortdate_create() {
  shortdate_layer = text_layer_create(GRect(0, 67, WIDTH, 25));
  text_layer_set_text_color(shortdate_layer, GColorBlack);
  text_layer_set_text_alignment(shortdate_layer, GTextAlignmentCenter);
  text_layer_set_background_color(shortdate_layer, GColorWhite);
  text_layer_set_font(shortdate_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(shortdate_layer));
}
// --- end shortdate---

// --- date ---
static char date_text[] = "Tue, Sep 30";
TextLayer *date_layer;

static void update_date(struct tm *tick_time, TimeUnits units_changed) {
  strftime(date_text, sizeof(date_text), "%a, %b %d", tick_time);
  text_layer_set_text(date_layer, date_text);
}

void date_create() {
  date_layer = text_layer_create(GRect(0, 42, WIDTH, 25));
  text_layer_set_text_color(date_layer, GColorBlack);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  text_layer_set_background_color(date_layer, GColorWhite);
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
}
// --- end date ---

// --- bluetooth ---
void bluetooth_handler(bool connected) {
  if (!connected)
    vibes_double_pulse();
}
void bluetooth_create() {
  bluetooth_connection_service_subscribe(bluetooth_handler);
}
// --- end bluetooth ---

// --- time ---
static char time_text[] = "23:59";
TextLayer *time_layer;

static void update_time(struct tm *tick_time, TimeUnits units_changed) {
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
}
// --- end time ---


void handle_tick(struct tm *tick_time, TimeUnits units_changed) {

  if (units_changed & DAY_UNIT) {
    update_date(tick_time, units_changed);
    update_shortdate(tick_time, units_changed);
  }
  if (units_changed & MINUTE_UNIT)
    update_time(tick_time, units_changed);
  if (units_changed & SECOND_UNIT)
    update_battery_tick(tick_time, units_changed);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
}

void handle_init(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);
  window_layer = window_get_root_layer(window);

  time_create();
  date_create();
  shortdate_create();
  bluetooth_create();
  battery_create();

  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
  time_t now = time(NULL);
  handle_tick(localtime(&now), SECOND_UNIT | MINUTE_UNIT | DAY_UNIT);
}

int main(void) {
  handle_init();

  app_event_loop();

  handle_deinit();
}
