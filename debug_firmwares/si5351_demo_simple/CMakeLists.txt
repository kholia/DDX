cmake_minimum_required(VERSION 3.13)

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
#set(PICO_SDK_FETCH_FROM_GIT on)

# pico_sdk_import.cmake is a single file copied from this SDK
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project C CXX ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(hello_world
    main.cpp
    si5351.cpp
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(hello_world pico_stdlib hardware_adc hardware_dma pico_unique_id hardware_i2c hardware_pio)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(hello_world)

pico_enable_stdio_uart(hello_world 0)
pico_enable_stdio_usb(hello_world 1)
