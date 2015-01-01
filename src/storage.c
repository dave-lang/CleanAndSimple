#include <pebble.h>
#include "storage.h"
#include "main.h"

// No storage setup, set defaults
static void init_persist() {
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_INFO, "Configuring storage");
  #endif
    
  // Currently only storing versioning & vibrate
  persist_write_bool(KEY_VIBRATE, true);
  persist_write_int(STORAGE_VERSION_KEY, CURRENT_STORAGE_VERSION);
}

// Read in the persistent settings
struct PersistentStorage read_persist() {
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_INFO, "Reading from storage");
  #endif
  
  struct PersistentStorage store;
  store.vibrate = persist_read_bool(KEY_VIBRATE);
  
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_INFO, "Vibrate is %s", store.vibrate ? "true" : "false");
  #endif
  
  return store;
}

// Read in the persistent settings
void update_persist(struct PersistentStorage store) {
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_INFO, "Updating storage");
  #endif
  
  persist_write_bool(KEY_VIBRATE, store.vibrate);
}

// This will be more useful when there's multiple versions
struct PersistentStorage migrate_and_read_persist(void) {
  uint32_t version = persist_read_int(STORAGE_VERSION_KEY); // defaults to 0 if key is missing.

  if (version != CURRENT_STORAGE_VERSION) {
    // Not currently used, only initial version currently
    #if DEBUG
      APP_LOG(APP_LOG_LEVEL_WARNING, "Non-current storage version");
    #endif
      
    switch (version) {
      case 0:
        #if DEBUG
          APP_LOG(APP_LOG_LEVEL_WARNING, "No storage configured");
        #endif
        init_persist();
        break;
    }
  }
  
  return read_persist();
}