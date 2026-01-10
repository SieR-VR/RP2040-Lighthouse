#ifndef __SENSOR_H__
#define __SENSOR_H__

#include "defines.h"
#include "pico/types.h"

typedef struct sensor_info {
  uint8_t last_level;
  uint32_t time_elapsed;
} sensor_info_t;

extern sensor_info_t photosensor[SENSOR_COUNT];

typedef struct sensor_output {
  uint32_t sensor_id;
  uint8_t level;
  uint32_t pulse_width;
} sensor_output_t;

#endif // __SENSOR_H__
