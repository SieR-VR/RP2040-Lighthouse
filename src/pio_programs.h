#include "pico/stdlib.h"
#include "pico/types.h"

#include "hardware/pio.h"
#include "hardware/dma.h"

#include "defines.h"

extern PIO pio;

extern uint32_t sm_buffer[2][SM_DMA_COUNT][SM_BUFFER_SIZE];
extern volatile bool sm_buffer_full[SM_DMA_COUNT];

extern int32_t dma_channel[2][SM_DMA_COUNT];
extern dma_channel_config dma_config[2][SM_DMA_COUNT];

uint32_t setup_level_monitor_program(uint32_t sm);
uint32_t setup_time_stamp_program(uint32_t sm);

void isr0();
