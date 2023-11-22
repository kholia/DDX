cmake_minimum_required(VERSION 3.12)

# rest of your project
add_executable(usb_analog_microphone
    main.cpp
    usb_descriptors.c
    usb_microphone.cpp
    si5351.cpp
    Adafruit_NeoPixel.cpp
    hash.c
    nhash.c
    wspr_enc.c
)

pico_generate_pio_header(usb_analog_microphone ${CMAKE_CURRENT_LIST_DIR}/ws2812byte.pio)

target_include_directories(usb_analog_microphone PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(usb_analog_microphone PRIVATE tinyusb_device tinyusb_board pico_analog_microphone hardware_pio pico_bootsel_via_double_reset hardware_adc hardware_dma hardware_i2c hardware_pwm hardware_watchdog)

# enable usb output, disable uart output
pico_enable_stdio_usb(pico_analog_microphone 0)
pico_enable_stdio_uart(pico_analog_microphone 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(usb_analog_microphone)
