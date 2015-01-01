#pragma once

// Includes/excludes debugging code, mostly logging
#define DEBUG false

// App message & storage keys
#define KEY_VIBRATE 0
  
// General layout figures
#define MAX_WIDTH 144
  
// Time layout
#define TIME_BUFFER_SIZE 6
#define TIME_FORMAT "%H:%M"
#define TIME_DEFAULT "00:00"
#define TIME_OFFSET 55
#define TIME_HEIGHT 50
  
// Date layout
#define DATE_BUFFER_SIZE 13 // Longest string - "September 31"
#define DATE_FORMAT "%B %d"
#define DATE_DEFAULT "September 31"
#define DATE_OFFSET 110
#define DATE_HEIGHT 30
  
// Battery layout
#define BATTERY_WIDTH 130
#define BATTERY_OFFSET 107
#define BATTERY_THICKNESS 4

// Bluetooth layout
#define BLUETOOTH_ICO_WIDTH 24
#define BLUETOOTH_ICO_HEIGHT 24
#define BLUETOOTH_ICO_OFFSET 1
#define BLUETOOTH_TEXT_HEIGHT 20
#define BLUETOOTH_TEXT_OFFSET 24