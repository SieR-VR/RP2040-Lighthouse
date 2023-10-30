#include <stdint.h>
#include <stdio.h>

#include "pico/multicore.h"
#include "pico/stdio_uart.h"
#include "pico/stdlib.h"
#include "pico/types.h"
#include "pico/util/queue.h"

#include "hardware/dma.h"

#include "defines.h"
#include "pio_programs.h"
#include "sensor.h"
#include "sensor_state_machine.h"

queue_t sm0_queue;
queue_t sm1_queue;
uint32_t sm0_offset, sm1_offset;

queue_t output_queue;

sensor_state_machine_t photosensor_sm[SENSOR_COUNT];
sweep_t last_sweep[SENSOR_COUNT];

void core0_setup();
void core0_loop();

void core1_setup();
void core1_loop();

void core1_main() {
  core1_setup();
  while (true) {
    core1_loop();
  }
}

int main() {
  multicore_launch_core1(core1_main);
  stdio_uart_init_full(uart0, 115200, 0, 1);

  core0_setup();
  while (true) {
    core0_loop();
  }
}

void core0_setup() {}

void core0_loop() {
  while (!queue_is_empty(&sm0_queue) && !queue_is_empty(&sm1_queue)) {
    uint32_t sm0_temp, sm1_temp, time_ns;
    queue_try_remove(&sm0_queue, &sm0_temp);
    queue_try_remove(&sm1_queue, &sm1_temp);
    time_ns = sm1_temp;

    for (uint32_t sensor_i = 0; sensor_i < SENSOR_COUNT; sensor_i++) {
      uint8_t bit_temp = (sm0_temp >> (31 - sensor_i)) & 1;
      photosensor[sensor_i].time_elapsed += time_ns;

      if (bit_temp != photosensor[sensor_i].last_level) {
        sensor_output_t output_temp = {
            .sensor_id = sensor_i,
            .level = photosensor[sensor_i].last_level,
            .pulse_width = photosensor[sensor_i].time_elapsed,
        };

        queue_try_add(&output_queue, &output_temp);
        photosensor[sensor_i].last_level = bit_temp;
        photosensor[sensor_i].time_elapsed = 0;
      }
    }
  }

  while (!queue_is_empty(&output_queue)) {
    sensor_output_t output_temp;
    queue_try_remove(&output_queue, &output_temp);

    int flag_sweep = 0;
    if (output_temp.level) {
      flag_sweep = sensor_sm_pulse_high(&photosensor_sm[output_temp.sensor_id],
                                        output_temp.pulse_width,
                                        &last_sweep[output_temp.sensor_id]);
    } else {
      sensor_sm_pulse_low(&photosensor_sm[output_temp.sensor_id],
                          output_temp.pulse_width);
    }

    if (flag_sweep) {
      static uint32_t static_sweep_tick[SENSOR_COUNT][2] = {0};
      static_sweep_tick[output_temp.sensor_id]
                       [last_sweep[output_temp.sensor_id].sweep_axis] =
                           last_sweep[output_temp.sensor_id].sweep_tick;

      printf("sensor_%d_x: %d, sensor_%d_y: %d\n", output_temp.sensor_id,
             static_sweep_tick[output_temp.sensor_id][0], output_temp.sensor_id,
             static_sweep_tick[output_temp.sensor_id][1]);
    }
  }
}

void core1_setup() {
  queue_init(&sm0_queue, sizeof(uint32_t), SM_QUEUE_LIMIT);
  queue_init(&sm1_queue, sizeof(uint32_t), SM_QUEUE_LIMIT);

  queue_init(&output_queue, sizeof(sensor_output_t), SM_QUEUE_LIMIT);

  sm0_offset = setup_level_monitor_program(0);
  sm1_offset = setup_time_stamp_program(1);

  for (uint32_t sm = 0; sm < 2; sm++)
    pio_sm_set_enabled(pio, sm, true);
}

void core1_loop() {
  for (uint32_t dma_i = 0; dma_i < SM_DMA_COUNT; dma_i++) {
    if (sm_buffer_full[dma_i] == true) {
      for (uint32_t data_i = 0; data_i < SM_BUFFER_SIZE; data_i++) {
        queue_try_add(&sm0_queue, &sm_buffer[0][dma_i][data_i]);
        queue_try_add(&sm1_queue, &sm_buffer[1][dma_i][data_i]);
      }
      sm_buffer_full[dma_i] = false;
    }
  }
}
