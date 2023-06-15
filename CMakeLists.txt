cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(PICO_SDK_FETCH_FROM_GIT on)

include(pico_sdk_import.cmake)

project(rp2040_lighthouse)
pico_sdk_init()

add_subdirectory(src)