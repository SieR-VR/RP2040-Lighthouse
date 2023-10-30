#include <stdint.h>

typedef enum {
  SENSOR_STATE_IDLE,
  SENSOR_STATE_X_PULSE,
  SENSOR_STATE_Y_PULSE,
} sensor_state;

typedef enum {
  SENSOR_X_SWEEP,
  SENSOR_Y_SWEEP,
} sensor_sweep_axis;

typedef struct {
  sensor_state state;
  uint32_t last_low_tick;
  uint32_t last_pulse_tick;
} sensor_state_machine_t;

typedef struct {
  uint8_t sensor_index;
  sensor_sweep_axis sweep_axis;
  uint32_t sweep_tick;
} sweep_t;

/**
 * Inputs high pulse to sensor
 * @param tick pulse tick, 1 tick = 92ns
 * @return 1 if was the pulse sweep, 0 otherwise
 */
int sensor_sm_pulse_high(sensor_state_machine_t *sm, uint32_t tick, sweep_t *sweep);

/**
 * Inputs low pulse to sensor
 * @return sweep_t* if was the pulse sweep, NULL otherwise
 */
void sensor_sm_pulse_low(sensor_state_machine_t *sm, uint32_t tick);
