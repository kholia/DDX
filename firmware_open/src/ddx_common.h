#ifndef DDX_common
#define DDX_common

#define VERSION        "1.0"
#define BUILD          1

#include <stdio.h>
#include <stdint.h>

#include "si5351.h"

#include "hardware/watchdog.h"
// #include <EEPROM.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"
#include "hardware/pwm.h"
#include "pico/multicore.h"
#include "hardware/adc.h"
#include "hardware/uart.h"

// Pinout
#define PTT            8 // PTT pin
#define CAL            15 // Automatic calibration entry
#define TXLED          3
#define GPS            1
#define SDA_PIN        12
#define SCL_PIN        13
#define ATU_PIN        14

// Buttons
#define ABORT_BTN      0 // repurposing the UART_TX pin
#define TUNE_BTN       2

extern uint64_t freq;
extern int32_t cal_factor;

void serialEvent();
void led_flash();

#endif
