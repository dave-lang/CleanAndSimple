#pragma once
#include <pebble.h>
  
#define STORAGE_VERSION_KEY 124 // any previously unused value

// Current storage versions
#define CURRENT_STORAGE_VERSION 1

typedef struct PersistentStorage
{
   bool vibrate;
} PersistentStorage; 

// Method signatures
struct PersistentStorage migrate_and_read_persist(void);
void update_persist(struct PersistentStorage store);