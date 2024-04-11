#include <pebble.h>

//Vector paths that define the watch face

/* ---------- Classic ---------- */

//Rectangular watches (aplite, basalt, diorite)
#if defined(PBL_RECT)
static const GPathInfo PATH_BLACK_CARET = {
  .num_points = 6,
  .points = (GPoint []) {{0, 6}, {52, 83}, {0, 162}, {0, 108}, {16, 83}, {0, 60}}
};
static const GPathInfo PATH_JEWEL = {
  .num_points = 6,
  .points = (GPoint []) {{-4, 0}, {148, 0}, {91, 83}, {148, 168}, {-4, 168}, {52, 83}}
};
static const GPathInfo PATH_JEWELSTROKE = {
  .num_points = 3,
  .points = (GPoint []) {{0, 6}, {52, 83}, {0, 162}}
};
#endif

//Round watches (chalk)
#if defined(PBL_ROUND)
static const GPathInfo PATH_BLACK_CARET = {
  .num_points = 6,
  .points = (GPoint []) {{-22, 6}, {14, 6}, {70, 89}, {14, 174}, {-22, 174}, {34, 89}}
};
static const GPathInfo PATH_JEWEL = {
  .num_points = 6,
  .points = (GPoint []) {{10, 0}, {169, 0}, {109, 89}, {169, 180}, {10, 180}, {70, 89}}
};
static const GPathInfo PATH_JEWELSTROKE = {
  .num_points = 3,
  .points = (GPoint []) {{14, 6}, {70, 89}, {14, 174}}
};
#endif