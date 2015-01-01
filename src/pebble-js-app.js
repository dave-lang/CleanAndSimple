var initialized = false;
var options = {};

var DEBUG = false;
var VIBRATE_KEY = "vibrate-disconnect";
var STORAGE_VERSION_KEY = "storage-version";
var CURRENT_STORAGE_VERSION = 1;

function read_and_migrate_storage() {
  if (DEBUG) console.log("reading storage");
  var currentVersion = localStorage.getItem(STORAGE_VERSION_KEY);
  
  if (currentVersion != CURRENT_STORAGE_VERSION) {
    console.warn("JS Storage not current");
    
    // Currently there's no other versions
    if (currentVersion === null) {
      localStorage.setItem(STORAGE_VERSION_KEY, CURRENT_STORAGE_VERSION);
      localStorage.setItem(VIBRATE_KEY, true);
    }
  }
  
  options[VIBRATE_KEY] = localStorage.getItem(VIBRATE_KEY) === 'true' ? 'on' : 'off';
}

Pebble.addEventListener("ready", function() {
  if (DEBUG) console.log("ready called!");
  read_and_migrate_storage();
  initialized = true;
});

Pebble.addEventListener("showConfiguration", function() {
  if (DEBUG) console.log("showing configuration");
  Pebble.openURL('http://davoaust.github.io/CleanAndSimple?'+encodeURIComponent(JSON.stringify(options)));
});

Pebble.addEventListener("webviewclosed", function(e) {
  if (DEBUG) console.log("configuration closed");
  // webview closed
  //Using primitive JSON validity and non-empty check
  if (e.response.charAt(0) == "{" && e.response.slice(-1) == "}" && e.response.length > 5) {
    options = JSON.parse(decodeURIComponent(e.response));
    if (DEBUG) console.log("Options = " + JSON.stringify(options));
    
    var vibrate = options[VIBRATE_KEY] === 'on';
    localStorage.setItem(VIBRATE_KEY, vibrate);
    
    Pebble.sendAppMessage( { "vibrate-disconnect": (vibrate ? 1 : 0) },
      function(e) {
        if (DEBUG) console.log('Successfully delivered message with transactionId=' + e.data.transactionId);
      },
      function(e) {
        if (DEBUG) console.log('Unable to deliver message with transactionId=' + e.data.transactionId + ' Error is: ' + e.error.message);
      }
    );

  } else {
    if (DEBUG) console.log("Cancelled");
  }
});