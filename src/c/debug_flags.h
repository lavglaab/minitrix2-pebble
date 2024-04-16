#include <pebble.h>

// Here's a convenience function that wraps APP_LOG, and 
// adds a boolean condition. I can use this to enable and
// disable various types of logging via defines in this file.
#define LOG_IF_ENABLED(condition, level, fmt, args...) if (condition) { APP_LOG(level, fmt, ## args); }

#define DEBUG_LOG_TIMESTRINGS false // Print time strings and substrings
#define DEBUG_LOG_LIFECYCLE false // Life cycle events like window creation and function calls
#define DEBUG_LOG_WEATHER true // Communication, processing, and storage relating to weather

#define DEBUG_UI_DUMMYMODE false // Replace time and data display for store screenshots