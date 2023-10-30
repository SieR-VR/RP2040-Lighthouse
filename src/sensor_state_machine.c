#include "sensor_state_machine.h"
#include <stdint.h>

typedef enum { SWEEP, PULSE, OTHER } tick_length_kind;

tick_length_kind helper_categorize_tick_length(uint32_t tick) {
	if (tick < 600) { // 55.2 us
		return SWEEP;
	} else if (tick < 2000) { // 184 us
		return PULSE;
	} else {
		return OTHER;
	}
}

/**
 * @brief Parse the pulse length from a pulse tick.
 * @param tick The pulse tick to parse.
 * @return only last 3 bits used,
 * 0x01: axis bit. 0 for x, 1 for y.
 * 0x02: ootx data bit. not used now.
 * 0x04: skip data bit. 1 for skip, 0 for not skip. (base station b mode or c mode)
 */
uint8_t helper_parse_pulse_length(uint32_t tick) {
	return ((tick - 616) / 112) & 0x07; // 616 = 56.6 us, 112 = 10.4 us
}

int sensor_sm_pulse_high(sensor_state_machine_t *sm, uint32_t tick, sweep_t *sweep) {
  tick_length_kind kind = helper_categorize_tick_length(tick);
	uint8_t pulse_data;

	switch (kind) {
		case SWEEP:
			if (sm->state == SENSOR_STATE_IDLE)
				break;

			sweep->sweep_axis = sm->state == SENSOR_STATE_X_PULSE ? SENSOR_X_SWEEP : SENSOR_Y_SWEEP;
			sweep->sweep_tick = sm->last_low_tick + sm->last_pulse_tick;

			sm->state = SENSOR_STATE_IDLE;
			return 1;
		case PULSE:
			pulse_data = helper_parse_pulse_length(tick);
			sm->last_pulse_tick = tick;

			if (pulse_data & 0x01) {
				sm->state = SENSOR_STATE_Y_PULSE;
			} else {
				sm->state = SENSOR_STATE_X_PULSE;
			}

			break;
		case OTHER:
			sm->state = SENSOR_STATE_IDLE;
			break;
	}

	return 0;
}

void sensor_sm_pulse_low(sensor_state_machine_t *sm, uint32_t tick) {
  int flag_normal_low_pulse = (tick <= 200000); // 16.6ms

	if (flag_normal_low_pulse) {
		sm->last_low_tick = tick;
	} else {
		sm->state = SENSOR_STATE_IDLE;
		sm->last_low_tick = 0;
	}
}

