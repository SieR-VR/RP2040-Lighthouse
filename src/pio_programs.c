#include "pio_programs.h"

#include "pico/stdlib.h"
#include "pico/types.h"

#include "ts4231.pio.h"
#include <stdio.h>

PIO pio = pio0;

const uint32_t clkdiv = 4;
const uint32_t in_base = 8, sideset_base = 28, jmp_pin = 15;

uint32_t sm_buffer[2][SM_DMA_COUNT][SM_BUFFER_SIZE];
volatile bool sm_buffer_full[SM_DMA_COUNT];

int32_t dma_channel[2][SM_DMA_COUNT];
dma_channel_config dma_config[2][SM_DMA_COUNT];

uint32_t setup_level_monitor_program(uint32_t sm) {
  uint32_t sm_offset = pio_add_program(pio, &level_monitor_program);
  pio_sm_config config = level_monitor_program_get_default_config(sm_offset);

  sm_config_set_clkdiv(&config, clkdiv);
  sm_config_set_in_pins(&config, in_base);

  sm_config_set_sideset(&config, 1, false, false);
  sm_config_set_sideset_pins(&config, sideset_base);

  pio_gpio_init(pio, sideset_base);
  pio_sm_set_consecutive_pindirs(pio, sm, sideset_base, 1, true);

  sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_RX);
  pio_sm_init(pio, sm, sm_offset, &config);

  for (uint32_t dma_i = 0; dma_i < SM_DMA_COUNT; dma_i++) {
    dma_channel[sm][dma_i] = dma_claim_unused_channel(true);
    dma_config[sm][dma_i] =
        dma_channel_get_default_config(dma_channel[sm][dma_i]);

    channel_config_set_read_increment(&dma_config[sm][dma_i], false);
    channel_config_set_write_increment(&dma_config[sm][dma_i], true);

    channel_config_set_dreq(&dma_config[sm][dma_i],
                            pio_get_dreq(pio, sm, false));
    channel_config_set_chain_to(
        &dma_config[sm][dma_i],
        dma_channel[sm][dma_i == SM_DMA_COUNT - 1 ? 0 : dma_i + 1]);

    dma_channel_set_irq0_enabled(dma_channel[sm][dma_i], true);
	}

	irq_set_exclusive_handler(DMA_IRQ_0, isr0);
  irq_set_enabled(DMA_IRQ_0, true);

	for (uint32_t dma_i = 0; dma_i < SM_DMA_COUNT; dma_i++) {
    dma_channel_configure(dma_channel[sm][dma_i], &dma_config[sm][dma_i],
                          sm_buffer[sm][dma_i], &pio->rxf[sm], SM_BUFFER_SIZE,
                          dma_i == 0);
  }

  return sm_offset;
}

uint32_t setup_time_stamp_program(uint32_t sm) {
  uint32_t sm_offset = pio_add_program(pio, &time_stamp_program);
  pio_sm_config config = time_stamp_program_get_default_config(sm_offset);

  sm_config_set_clkdiv(&config, clkdiv);
  sm_config_set_jmp_pin(&config, jmp_pin);

  sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_RX);
  pio_sm_init(pio, sm, sm_offset, &config);

  for (uint32_t dma_i = 0; dma_i < SM_DMA_COUNT; dma_i++) {
    dma_channel[sm][dma_i] = dma_claim_unused_channel(true);
    dma_config[sm][dma_i] =
        dma_channel_get_default_config(dma_channel[sm][dma_i]);

    channel_config_set_read_increment(&dma_config[sm][dma_i], false);
    channel_config_set_write_increment(&dma_config[sm][dma_i], true);

    channel_config_set_dreq(&dma_config[sm][dma_i],
                            pio_get_dreq(pio, sm, false));
    channel_config_set_chain_to(
        &dma_config[sm][dma_i],
        dma_channel[sm][(dma_i == (SM_DMA_COUNT - 1)) ? 0 : dma_i + 1]);

    dma_channel_set_irq0_enabled(dma_channel[sm][dma_i], true);
  }

	for (uint32_t dma_i = 0; dma_i < SM_DMA_COUNT; dma_i++) {
    dma_channel_configure(dma_channel[sm][dma_i], &dma_config[sm][dma_i],
                          sm_buffer[sm][dma_i], &pio->rxf[sm], SM_BUFFER_SIZE,
                          dma_i == 0);
  }

  return sm_offset;
}

void isr0() {
  for (uint32_t dma_i = 0; dma_i < SM_DMA_COUNT; dma_i++) {
    if (dma_channel_get_irq0_status(dma_channel[0][dma_i]) &&
        dma_channel_get_irq0_status(dma_channel[1][dma_i])) {
      sm_buffer_full[dma_i] = true;
      dma_channel_configure(dma_channel[0][dma_i], &dma_config[0][dma_i],
                            sm_buffer[0][dma_i], &pio->rxf[0], SM_BUFFER_SIZE,
                            dma_i == 0);
      dma_channel_configure(dma_channel[1][dma_i], &dma_config[1][dma_i],
                            sm_buffer[1][dma_i], &pio->rxf[1], SM_BUFFER_SIZE,
                            dma_i == 0);
      dma_channel_acknowledge_irq0(dma_channel[0][dma_i]);
      dma_channel_acknowledge_irq0(dma_channel[1][dma_i]);
    }
  }
}
