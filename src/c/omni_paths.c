#include <pebble.h>

//Vector paths that define the watch face

/* ---------- Omniverse ---------- */

//Rectangular watches (aplite, basalt, diorite)
#if defined(PBL_RECT)
static const GPathInfo PATH_GREEN_STRIPE = {
  .num_points = 8,
  .points = (GPoint []) {{0, 0}, {40, 0}, {61, 84}, {40, 168}, {0, 168}, {0, 105}, {35, 84}, {0,62}}
};
static const GPathInfo PATH_INNER_MASK = {
  .num_points = 6,
  .points = (GPoint []) {{40, 0}, {103, 0}, {82, 84}, {102, 168}, {40, 168}, {61, 84}}
};
static const GPathInfo PATH_STRIPE_STROKE_INNER = {
  .num_points = 3,
  .points = (GPoint []) {{40, 0}, {61, 84}, {40, 168}}
};
static const GPathInfo PATH_STRIPE_STROKE_OUTER = {
  .num_points = 3,
  .points = (GPoint []) {{0, 105}, {35, 84}, {0,62}}
};
#endif

//Round watches (chalk)
#if defined(PBL_ROUND)
static const GPathInfo PATH_GREEN_STRIPE = {
  .num_points = 8,
  .points = (GPoint []) {{18, 6}, {56, 0}, {79, 90}, {56, 180}, {18, 174}, {0, 122}, {53, 90}, {0, 57}}
};
static const GPathInfo PATH_INNER_MASK = {
  .num_points = 6,
  .points = (GPoint []) {{56, 0}, {122, 0}, {100, 90}, {122, 180}, {56, 180}, {79, 90}}
};
static const GPathInfo PATH_STRIPE_STROKE_INNER = {
  .num_points = 3,
  .points = (GPoint []) {{56, 0}, {79, 90}, {56, 180}}
};
static const GPathInfo PATH_STRIPE_STROKE_OUTER = {
  .num_points = 3,
  .points = (GPoint []) {{0, 122}, {53, 90}, {0, 57}}
};
#endif