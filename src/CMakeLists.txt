add_executable(rp2040_lighthouse
    main.c
    sensor.c
    pio_programs.c
    sensor_state_machine.c
)

target_link_libraries(rp2040_lighthouse PRIVATE
    pico_stdlib
    hardware_pio
    hardware_dma
    pico_multicore
)

add_subdirectory(pio)

pico_enable_stdio_usb(rp2040_lighthouse 1)
# pico_enable_stdio_uart(rp2040_lighthouse 0)

pico_add_extra_outputs(rp2040_lighthouse)
