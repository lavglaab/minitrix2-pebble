#include <pebble.h>

// Here, we'll define a percentage threshold for when the
// watchface should indicate the battery is low. I'm
// defining this separately per platform, as the
// many makes and models of Pebble-like smartwatch have
// a few different classes of expected battery life.
//
// I want to define "low battery" as between 0.5--1 days left

#define _LOW_BATTERY_30D (6)
#define _LOW_BATTERY_14D (8)
#define _LOW_BATTERY_7D (20)
#define _LOW_BATTERY_2D (40)

// and the default definition for current platform
#if defined(PBL_PLATFORM_GABBRO)
    // ~14 days of marketed battery life
    #define LOW_BATTERY_SELF _LOW_BATTERY_14D
#elif defined(PBL_PLATFORM_FLINT) || defined(PBL_PLATFORM_EMERY)
    // 30 days marketed, IME closer to 10 or 11
    #define LOW_BATTERY_SELF _LOW_BATTERY_30D
#elif defined(PBL_PLATFORM_CHALK)
    // 2 days battery when it was new
    #define LOW_BATTERY_SELF _LOW_BATTERY_2D
#else
    // probably a rectangular Pebble Tech Corp watch
    // Technically bobbysmiles has a larger battery than
    // snowy, but i am ignoring that. shush
    #define LOW_BATTERY_SELF _LOW_BATTERY_7D
#endif
