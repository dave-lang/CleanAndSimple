#include <pebble.h>
#include <math.h>
#include "main.h"

Window *s_main_window;
TextLayer *s_time_layer;
TextLayer *s_date_layer;
BitmapLayer *s_battery_image_layer;
BitmapLayer *s_bluetooth_layer;
TextLayer *s_bluetooth_text_layer;
GBitmap *bt_image;
GBitmap *battery_image;

// Data
char timeBuffer[TIME_BUFFER_SIZE];
char dateBuffer[DATE_BUFFER_SIZE];
char blueBuffer[14];
int batteryPerc = 0;
bool bluetoothConn;
// State variables
bool vibr_on = false;

// Thanks to https://github.com/Jnmattern/Minimalist/blob/master/src/bitmap.h for bmp drawing
static inline void bmpPutPixel(GBitmap *bmp, int x, int y, GColor c) {
  if (x >= bmp->bounds.size.w || y >= bmp->bounds.size.h || x < 0 || y < 0) return;
  int byteoffset = y*bmp->row_size_bytes + x/8;
  ((uint8_t *)bmp->addr)[byteoffset] &= ~(1<<(x%8));
  if (c == GColorWhite) ((uint8_t *)bmp->addr)[byteoffset] |= (1<<(x%8));
}

static inline void bmpRect(GBitmap *bmp, GRect rect, GColor c, bool fill) {
  int i, j;
  int xe = rect.origin.x + rect.size.w;
  int ye = rect.origin.y + rect.size.h;
  for (j=rect.origin.y ; j<ye; j++) {
    for (i=rect.origin.x ; i<xe; i++) {
      if (fill || (!(j > rect.origin.y && j < (ye - 1)) || (i == rect.origin.x || i == (xe - 1)))) {
        bmpPutPixel(bmp, i, j, c);
      }
    }
  }
}

void refresh_battery_layer() {
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_INFO, "refresh_battery_layer");
  #endif
    
  int batteryChargedWidth = floor(batteryPerc * BATTERY_WIDTH / 100);
  
  battery_image = gbitmap_create_blank(GSize(BATTERY_WIDTH, BATTERY_THICKNESS));
  bmpRect(battery_image, GRect(0, 0, BATTERY_WIDTH, BATTERY_THICKNESS), GColorWhite, false);
  bmpRect(battery_image, GRect(0, 0, batteryChargedWidth, BATTERY_THICKNESS), GColorWhite, true); 
  bitmap_layer_set_bitmap(s_battery_image_layer, battery_image);
  
  layer_mark_dirty(bitmap_layer_get_layer(s_battery_image_layer));
}

void refresh_bluetooth_layer() {
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_INFO, "refresh_bluetooth_layer"); 
  #endif
    
  if (!bluetoothConn) {
    if (bt_image == NULL)
      bt_image = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_ICON);
    
    bitmap_layer_set_bitmap(s_bluetooth_layer, bt_image);
    
    if (vibr_on)
      vibes_long_pulse();

    strncpy(blueBuffer, "No connection", 14);
    text_layer_set_text(s_bluetooth_text_layer, blueBuffer);

  } else {
    bitmap_layer_set_bitmap(s_bluetooth_layer, NULL);

    strncpy(blueBuffer, "", 14);
    text_layer_set_text(s_bluetooth_text_layer, blueBuffer);
    
    if (vibr_on)
      vibes_long_pulse();
  }
  
  layer_mark_dirty(bitmap_layer_get_layer(s_bluetooth_layer));
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_INFO, "tick_handler");  
  #endif
  
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(timeBuffer, TIME_BUFFER_SIZE, TIME_FORMAT, tick_time);
  } else {
    // Use 12 hour format
    strftime(timeBuffer, TIME_BUFFER_SIZE, TIME_FORMAT, tick_time);
  }
  strftime(dateBuffer, DATE_BUFFER_SIZE, DATE_FORMAT, tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, timeBuffer);
  // Display this date on the TextLayer
  text_layer_set_text(s_date_layer, dateBuffer);
}

void battery_handler(BatteryChargeState charge) {
  batteryPerc = charge.charge_percent;
  refresh_battery_layer();
}

void bluetooth_handler(bool connected) {
  bluetoothConn = connected;
  refresh_bluetooth_layer();
}

void main_window_load(Window *window) {
  window_set_background_color(window, GColorBlack);
  
  /// Time layer
  s_time_layer = text_layer_create(GRect(0, TIME_OFFSET, MAX_WIDTH, TIME_HEIGHT));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorClear);
  text_layer_set_text(s_time_layer, TIME_DEFAULT);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  /// Date layer
  s_date_layer = text_layer_create(GRect(0, DATE_OFFSET, MAX_WIDTH, DATE_HEIGHT));
  text_layer_set_background_color(s_date_layer, GColorBlack);
  text_layer_set_text_color(s_date_layer, GColorClear);
  text_layer_set_text(s_date_layer, DATE_DEFAULT);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  /// Battery bar, centered
  s_battery_image_layer = bitmap_layer_create(GRect((MAX_WIDTH - BATTERY_WIDTH) / 2, BATTERY_OFFSET, BATTERY_WIDTH, BATTERY_THICKNESS));
  
  // Bluetooth icon layer, centered
  s_bluetooth_layer = bitmap_layer_create(GRect((MAX_WIDTH - BLUETOOTH_ICO_WIDTH) / 2, BLUETOOTH_ICO_OFFSET, BLUETOOTH_ICO_WIDTH, BLUETOOTH_ICO_HEIGHT));
  
  // Bluetooth text layer
  s_bluetooth_text_layer = text_layer_create(GRect(0, BLUETOOTH_TEXT_OFFSET, MAX_WIDTH, BLUETOOTH_TEXT_HEIGHT));
  text_layer_set_background_color(s_bluetooth_text_layer, GColorBlack);
  text_layer_set_text_color(s_bluetooth_text_layer, GColorClear);
  text_layer_set_text(s_bluetooth_text_layer, "");
  
  text_layer_set_font(s_bluetooth_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_bluetooth_text_layer, GTextAlignmentCenter);
  
  // Add child layers to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_battery_image_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bluetooth_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bluetooth_text_layer));
}

void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  bitmap_layer_destroy(s_battery_image_layer);
  bitmap_layer_destroy(s_bluetooth_layer);
  text_layer_destroy(s_bluetooth_text_layer);
  gbitmap_destroy(bt_image);
  gbitmap_destroy(battery_image);
}

void init() {
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_INFO, "init");
  #endif
    
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
  bluetooth_connection_service_subscribe(bluetooth_handler);
  
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, false);
    
  // Make sure the time is displayed from the start
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  tick_handler(tick_time, MINUTE_UNIT);
  
  // Update battery
  battery_handler(battery_state_service_peek());
  
  // Update bluetooth
  bluetooth_handler(bluetooth_connection_service_peek()); 
  
  // Enable vibrate for events
  vibr_on = true;
}

void deinit() {
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_INFO, "deinit");
  #endif
    
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}