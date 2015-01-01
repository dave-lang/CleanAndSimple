#include <pebble.h>
#include <math.h>
#include "main.h"
#include "storage.h"
#include "drawing.h"

struct PersistentStorage *store;

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
// State
bool setupFinished = false;

void message_received_handler(DictionaryIterator *received, void *context) {
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_INFO, "App Message Received!");
  #endif
    
  // Get the first pair
  Tuple *t = dict_read_first(received);

  // Process all pairs present
  while(t != NULL) {
    // Process this pair's key
    switch (t->key) {
      case KEY_VIBRATE:
      APP_LOG(APP_LOG_LEVEL_WARNING, "key %lu", t->key);
      
        #if DEBUG
          APP_LOG(APP_LOG_LEVEL_INFO, "KEY_VIBRATE received with value %d", (int)t->value->int32);
        #endif
          
        // Set vibrate setting
        store.vibrate = ((int)t->value->int32 == 1);
        update_persist(store);
      
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_WARNING, "Unrecognised key %lu", t->key);
    }

    // Get next pair, if any
    t = dict_read_next(received);
  }
}

void message_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "App Message Dropped!");
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
    
    if (setupFinished && store.vibrate)
      vibes_long_pulse();

    strncpy(blueBuffer, "No connection", 14);
    text_layer_set_text(s_bluetooth_text_layer, blueBuffer);

  } else {
    bitmap_layer_set_bitmap(s_bluetooth_layer, NULL);

    strncpy(blueBuffer, "", 14);
    text_layer_set_text(s_bluetooth_text_layer, blueBuffer);
    
    if (setupFinished && store.vibrate)
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
  
  store = migrate_and_read_persist();
  
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
  
  // Configure handlers to receive settings from phone
  app_message_register_inbox_received(message_received_handler);
  app_message_register_inbox_dropped(message_dropped_handler);
  app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
  
  // Finished all setup, used to check for vibrate
  setupFinished = false;
}

void deinit() {
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_INFO, "deinit");
  #endif
  
  // Unsubscribe from all services
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  app_message_deregister_callbacks();
  
  // Destroy the window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}