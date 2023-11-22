// DDX-v3 firmware

#include <string>
#include <ostream>

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "analog_microphone.h"
#include "pico/multicore.h"

#include "usb_microphone.h"
#include "tusb.h"

#include "ddx_common.h"
#include "si5351.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "hash.h"

#include "hardware/i2c.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"

// ft8_lib stuff
#include "../ft8_lib/ft8/message.h"
#include "../ft8_lib/ft8/encode.h"
#include "../ft8_lib/ft8/constants.h"

#include "wspr_enc.h"

int symbolCount = 79;
int toneSpacing = 625;
#define FT8_TONE_SPACING 625  // ~6.25 Hz
#define FT8_DELAY 159  // Delay value for FT8
#define FT8_SYMBOL_COUNT 79
uint8_t tones[FT8_SYMBOL_COUNT];
int toneDelay = 159;

// #define DEBUG_ON 1

// CAT code
static uint8_t insideCat = 0;
unsigned char doingCAT = 0;
bool txCAT             = false;  // turned on if the transmitting due to a CAT command
bool message_available = false;
#define CAT_MAX 24
static char cat[CAT_MAX + 64];   // 64 avoids overflow
uint16_t offset = 1200;
static int sidx = 0;
char current_message[32];
char buffer[32];

#include "pico/stdio.h"
#include "pico/time.h"
#include "Adafruit_NeoPixel.hpp"

#define ONBOARD 16 // GP16 has 1 pixel NEOPIXEL strand connected to it

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel onboard = Adafruit_NeoPixel(1, ONBOARD, NEO_RGB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.
// additional functions
void colorWipe(uint32_t c, uint8_t wait);

// configuration
const struct analog_microphone_config config = {
  // GPIO to use for input, must be ADC compatible (GPIO 26 - 28)
  .gpio = 26,
  // bias voltage of microphone in volts
  .bias_voltage = 1.65, // 3.3v / 2
  // sample rate in Hz
  .sample_rate = 16000,
  // number of samples to buffer
  .sample_buffer_size = SAMPLE_BUFFER_SIZE,
};

// variables
int16_t sample_buffer[SAMPLE_BUFFER_SIZE];
volatile int samples_read = 0;

void on_analog_samples_ready()
{
  samples_read = analog_microphone_read(sample_buffer, SAMPLE_BUFFER_SIZE);
}

void on_usb_microphone_tx_ready()
{
  usb_microphone_write(sample_buffer, sizeof(sample_buffer));
}

void cdc_task(void);

// Automatic calibration stuff
uint32_t f_hi_;
void pwm_int_() {
  pwm_clear_irq(7);
  f_hi_++;
}

void band_change(int idx)
{
  si5351_set_freq((freq) * 100ULL, SI5351_CLK1); // CLK1 is RX clock
}

/* Button handling code */
int btnDown(int btn) {
  if (gpio_get(btn) == 1)
    return 0;
  else
    return 1;

  return 0;
}

static void str_to_upper(char *dist, const char *str)
{
  while (*str) *dist++ = toupper(*str++);
  *dist = 0;
}

int wspr_encode(char *call, char *grid, char *dBm, uint8_t *symbols)
{
  char call_u[11], grid_u[7];

  // Call
  if ((strlen(call) < 2) || (strlen(call) > 10)) {
    printf("The length of callsign %s is invalid.\n", call);
    return -1;
  }
  str_to_upper(call_u, call);
  // for Swaziland 3DA0YZ, 3D0YZ will be used for proper decoding
  if (strncmp(call_u, "3DA0", 4) == 0) {
    size_t i;
    for (i = 3; i < strlen(call); i++) {
      call[i - 1] = call[i];
    }
    call[i - 1] = 0;
  }
  str_to_upper(call_u, call);

  // Grid
  if ((strlen(grid) != 4) && (strlen(grid) != 6)) {
    printf("The length of grid locator %s is invalid.\n", grid);
    return -1;
  }
  str_to_upper(grid_u, grid);

  printf("WSPR Message: %s %s %s\n", call, grid, dBm);

  // invoke encoder function to generate symbols
  uint8_t msgtype = wspr_enc(call, grid, dBm, symbols);

  /* printf("Channel symbols: (message type %d)", msgtype);
    int i;
    for (i = 0; i < 162; i++) {
      if (i%30 == 0) {
          printf("\n      %d", symbols[i]);
      } else {
          printf(" %d", symbols[i]);
      }
    }
    printf("\n"); */

  return 0;
}

void active_sleep_ms(unsigned int delay_by) {
  sleep_ms(delay_by);

  /* unsigned long timeStart = millis();
    while (millis() - timeStart <= delay_by) {
    // Background Work
    } */
}

int checkButton(int btn) {
  int ret = 0;

  // only if the button is pressed
  if (!btnDown(btn))
    return ret;
  active_sleep_ms(50);
  if (!btnDown(btn)) // debounce
    return ret;

  ret = 1;
  if (btn == ABORT_BTN) {
    gpio_put(PTT, 0);
    si5351_set_clock_pwr(SI5351_CLK0, 0);
    si5351_output_enable(SI5351_CLK0, 0);
  } else if (btn == TUNE_BTN) {
    gpio_put(ATU_PIN, 1);
    // Turn on the output
    gpio_put(PTT, 1);
    sleep_ms(12); // Important delay for relay switching (for OEN 46 Series)
    si5351_set_freq(((freq + offset) * 100ULL), SI5351_CLK0);
    si5351_set_clock_pwr(SI5351_CLK0, 1);
    si5351_output_enable(SI5351_CLK0, 1);
    colorWipe(onboard.Color(255, 0, 0), 0); // R
    sleep_ms(3500); // Full tune cycle (LDG Z-100 Plus Automatic Tuner)
    // Turn off the output
    si5351_set_clock_pwr(SI5351_CLK0, 0);
    si5351_output_enable(SI5351_CLK0, 0);
    gpio_put(PTT, 0);
    // colorWipe(onboard.Color(0, 255, 0), 0); // G
    colorWipe(onboard.Color(0, 0, 0), 0); // Off

    gpio_put(ATU_PIN, 0);
  }

  // wait for the button to go up again
  while (btnDown(btn))
    active_sleep_ms(10);
  active_sleep_ms(50);

  return ret;
}

bool start_tx = false;

void tx()
{
  uint8_t i;

  gpio_put(PTT, 1);
  sleep_ms(12); // Important delay for relay switching (for OEN 46 Series)
#ifndef DEBUG_ON
  si5351_set_clock_pwr(SI5351_CLK0, 1);
  si5351_output_enable(SI5351_CLK0, 1);
  si5351_drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); // Set max. power for TX (UCC driver)
  colorWipe(onboard.Color(255, 0, 0), 0); // R
#endif
  for (i = 0; i < symbolCount; i++) {
#ifndef DEBUG_ON
    si5351_set_freq(((freq + offset) * 100ULL) + (tones[i] * toneSpacing), SI5351_CLK0);
#endif
    sleep_ms(toneDelay);
  }

  // Turn off the output
#ifndef DEBUG_ON
  si5351_set_clock_pwr(SI5351_CLK0, 0);
  si5351_output_enable(SI5351_CLK0, 0);
#endif
  gpio_put(PTT, 0);
#ifndef DEBUG_ON
  si5351_set_freq((freq) * 100ULL, SI5351_CLK1); // CLK1 is RX clock, reinit rx_clock_line?
  colorWipe(onboard.Color(0, 0, 0), 0); // Off
#endif
}

void core1_entry() {
  int ret;
  while (1) {
    // checkButton(ABORT_BTN);
    // checkButton(TUNE_BTN);
    if (start_tx) {
      tx();
      start_tx = false;
    }
    sleep_ms(10);
  }
}

void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < onboard.numPixels(); i++) {
    onboard.setPixelColor(i, c);
    onboard.show();
    sleep_ms(wait);
  }
}

void crash_with_led()
{
  while (1) {
    colorWipe(onboard.Color(255, 0, 0), 100); // Red
    colorWipe(onboard.Color(0, 255, 0), 100); // Green
    colorWipe(onboard.Color(0, 0, 255), 100); // Blue
  }
}

void finite_crash_with_led()
{
  int i = 7;

  while (i >= 0) {
    colorWipe(onboard.Color(255, 0, 0), 100); // Red
    colorWipe(onboard.Color(0, 255, 0), 100); // Green
    colorWipe(onboard.Color(0, 0, 255), 100); // Blue
    i = i - 1;
  }
}

static void hash_to_string(char string[65], const uint8_t hash[32])
{
  size_t i;
  for (i = 0; i < 32; i++) {
    string += sprintf(string, "%02x", hash[i]);
  }
}

void watchdog_task();

int main(void)
{
  int ret;

  stdio_init_all();

  // Safety first
  gpio_init(PTT);
  gpio_set_dir(PTT, GPIO_OUT);
  gpio_put(PTT, 0);

  // NeoPixel stuff
  onboard.begin();
  onboard.setBrightness(25);
  onboard.show(); // Initialize all pixels to 'off'

  // sleep_ms(5000);

#ifndef DEBUG_ON
  // Initialize the Si5351 --- this will go into an infinte loop in case of problems!
  while (1) {
    ret = si5351_init(0x60, SI5351_CRYSTAL_LOAD_8PF, 26000000, 0); // I am using a 26 MHz TCXO
    if (!ret) {
      // crash_with_led(); // Dhiru's hack
      // watchdog_reboot(0, 0, 1000);
      colorWipe(onboard.Color(255, 0, 0), 100); // Red
      sleep_ms(500);
      colorWipe(onboard.Color(255, 255, 0), 100);
      sleep_ms(500);
    }
    else {
      break;
    }
  }

  colorWipe(onboard.Color(0, 0, 0), 0); // Off
  // si5351_init(0x60, SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  si5351_output_enable(SI5351_CLK0, 0); // Turn of TX first - safety!
  // CLK1 is RX clock
  si5351_set_freq(freq * 100ULL, SI5351_CLK1);
  si5351_drive_strength(SI5351_CLK1, SI5351_DRIVE_4MA); // Set a bit reduced power for RX
  si5351_drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); // Set max. power for TX (UCC driver)
#endif

  // Initialize GPIOs
  gpio_init(TUNE_BTN);
  gpio_init(ABORT_BTN);
  gpio_set_dir(TUNE_BTN, GPIO_OUT);
  gpio_set_dir(ABORT_BTN, GPIO_OUT);
  gpio_pull_up(TUNE_BTN);
  gpio_pull_up(ABORT_BTN);
  gpio_set_dir(CAL, GPIO_IN);
  gpio_put(ATU_PIN, 0);

  // "Pico's" serial number
  int len = 16;
  char buff[len + 1] = "";
  pico_get_unique_board_id_string((char *)buff, len + 1);
#ifdef DEBUG_ON_VERBOSE
  printf(buff);
#endif

  uint8_t hash[32];
  char hash_string[65];
  calc_sha_256(hash, buff, len);
  hash_to_string(hash_string, hash);

  // Turn on RX
  gpio_put(PTT, 0);

  // EEPROM.get(4, cal_factor);
  // si5351_set_correction(cal_factor, SI5351_PLL_INPUT_XO);

  // Pico soundcard stuff
  if (analog_microphone_init(&config) < 0) {
    while (1) {
      tight_loop_contents();
    }
  }
  analog_microphone_set_samples_ready_handler(on_analog_samples_ready);
  if (analog_microphone_start() < 0) { // start capturing data from the analog microphone
    while (1) {
      tight_loop_contents();
    }
  }
  usb_microphone_init();
  usb_microphone_set_tx_ready_handler(on_usb_microphone_tx_ready);

  // note!
  multicore_launch_core1(core1_entry);

  while (1) {
    // run the USB microphone task continuously
    usb_microphone_task(); // calls tud_task() -> TinyUSB device task

    cdc_task(); // CAT handler -> should return fast!

    watchdog_task();
  }
  return 0;
}

// Variables
uint64_t freq = 28074000ULL; // Start on the 10m by default
uint64_t new_freq;
int32_t cal_factor = 0; // Sorted out by auto-calibration algorithm, now not required by tcxo ;)

absolute_time_t tx_timestamp;

void watchdog_task() {
  watchdog_update();

  if (start_tx == true) {
    // Another safeguard for FT8
    absolute_time_t current_timestamp = get_absolute_time();
    if (absolute_time_diff_us(tx_timestamp, current_timestamp) > 15000000ULL) {
      si5351_set_clock_pwr(SI5351_CLK0, 0);
      si5351_output_enable(SI5351_CLK0, 0);
      gpio_put(PTT, 0);
      finite_crash_with_led();
      watchdog_reboot(0, 0, 1000);
    }
  }
}

char buf[64];

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
// Dhiru -> Don't block in this task at all - it impacts the audio quality!
void cdc_task(void)
{
  if (tud_cdc_available())
  {
    memset(buf, 0, sizeof(buf));
    uint32_t count = tud_cdc_read(buf, sizeof(buf)); // count can be 1 here - so we have to piece 'strings' together ourselves
    // printf(count);
    // printf(sidx);

    memcpy(cat + sidx, buf, count);
    // printf(cat);
    cat[CAT_MAX - 1] = 0;
    sidx = (sidx + count) % CAT_MAX; // not perfect

    // Protocol -> a valid message starts with '*' and also ends with '*'
    char *s = strchr(cat, '*');
    char *e = strrchr(cat, '*');
    if (s != e && s != NULL && e != NULL ) { /* We may have a valid message */
      if (*(s + 1) == 'v') { // 'ready' debug
        char *response = (char *)"r";
        // Send indication for ready!
        tud_cdc_write(response, 1);
        tud_cdc_write_flush();
      } else if (*(s + 1) == 'o') {
        char *response = (char *)"o";
        offset = atoi(strtok(s + 2, "*"));
      } else if (*(s + 1) == 'm') { // message load
        char *response = (char *)"m";
        message_available = true;
        strncpy(current_message, strtok(s + 2, "*"), 32 - 1);
#ifdef DEBUG_ON_VERBOSE
        printf(current_message);
#endif
        ftx_message_t msg;
        ftx_message_rc_t rc = ftx_message_encode(&msg, NULL, current_message);
        bool is_ft4 = false;
        if (rc != FTX_MESSAGE_RC_OK) {
#ifdef DEBUG_ON_VERBOSE
          printf("Cannot parse message!\n");
          printf("RC = %d\n", (int)rc);
#endif
          colorWipe(onboard.Color(0, 255, 255), 0); // Blue-green (cyan)
          sleep_ms(1000);
        }
        int num_tones = (is_ft4) ? FT4_NN : FT8_NN;
        // Second, encode the binary message as a sequence of FSK tones
        if (is_ft4) {
          ft4_encode(msg.payload, tones);
        } else {
          ft8_encode(msg.payload, tones);
        }
      } else if (*(s + 1) == 't') { // tx on
        if (message_available) {
          char *response = (char *)"t";
          start_tx = true;
          watchdog_enable(7000, 1);
          tx_timestamp = get_absolute_time();
        }
      } else if (*(s + 1) == 'z') { // ptt off
        if (message_available) {
          char *response = (char *)"z";
#ifdef DEBUG_ON_VERBOSE
          printf("PTT OFF");
#endif
          gpio_put(PTT, 0);
        }
      } else if (*(s + 1) == 'p') { // ptt on
        if (message_available) {
          char *response = (char *)"p";
#ifdef DEBUG_ON_VERBOSE
          printf("PTT ON");
#endif
          // gpio_put(PTT, 1); // Disabled for safety
        }
      } else if (*(s + 1) == 'y') { // debug
        if (message_available) {
          char *response = (char *)"y";
        }
      } else if (*(s + 1) == 's') { // 'style' (digital mode)
        strncpy(buffer, strtok(s + 2, "*"), 32 - 1);
	puts(buffer);
      } else if (*(s + 1) == 'f') { // frequency
        char *response = (char *)"f";
        char *f = strtok(s + 2, "*");
        char *ptr;
        new_freq = strtoul(f, &ptr, 10);
        freq = new_freq;
        uint8_t band_idx;
        /* switch (new_freq) {
          case 7074000:
            band_idx = 0;
            break;
          case 14074000:
            band_idx = 1;
            break;
          case 21074000:
            band_idx = 2;
            break;
          case 28074000:
            band_idx = 3;
            break;
          } */
        // band_change(band_idx);
#ifndef DEBUG_ON
        si5351_set_freq((freq) * 100ULL, SI5351_CLK1); // CLK1 is RX clock
#endif
#ifdef DEBUG_ON_VERBOSE
        printf("Frequency switch - CAREFUL!");
#endif
      }
      // Clear stuff
      sidx = 0;
      memset(cat, 0, CAT_MAX * 2);
      // colorWipe(onboard.Color(0, 255, 0), 100); // Green
    }
  }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  (void) itf;
  (void) rts;

  if (dtr)
  {
    // Terminal connected
  } else
  {
    // Terminal disconnected
  }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
  (void) itf;
}
