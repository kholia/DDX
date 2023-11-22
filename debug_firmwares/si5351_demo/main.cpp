#include <math.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"

#include "si5351.h"

#include "pico/stdio.h"
#include "pico/time.h"
#include "Adafruit_NeoPixel.hpp"

#define ONBOARD 16 // GP16 has 1 pixel NEOPIXEL strand connected to it

Adafruit_NeoPixel onboard = Adafruit_NeoPixel(1, ONBOARD, NEO_GRB + NEO_KHZ800);

void colorWipe(uint32_t c, uint8_t wait);

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

int main(void)
{
  int ret;

  // NeoPixel stuff
  onboard.begin();
  onboard.setBrightness(50);
  onboard.show(); // Initialize all pixels to 'off'

  stdio_init_all();

  // Initialize the Si5351
  ret = si5351_init(0x60, SI5351_CRYSTAL_LOAD_8PF, 26000000, 0); // I am using a 26 MHz TCXO
  if (!ret) {
	  crash_with_led();
  }
  // si5351_init(0x60, SI5351_CRYSTAL_LOAD_8PF, 0, 0);

  // si5351_set_freq(7074000ULL * 100ULL, SI5351_CLK0);
  si5351_set_freq(21074000ULL * 100ULL, SI5351_CLK1); // RX clock, 15m band
  // si5351_set_freq(28074000ULL * 100ULL, SI5351_CLK2);

  return 0;
}
