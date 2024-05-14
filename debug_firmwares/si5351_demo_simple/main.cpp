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

int main(void)
{
  int ret;

  stdio_init_all();

  // Initialize the Si5351
  ret = si5351_init(0x60, SI5351_CRYSTAL_LOAD_8PF, 26000000, 0); // I am using a 26 MHz TCXO
  if (!ret) {
  }
  // si5351_init(0x60, SI5351_CRYSTAL_LOAD_8PF, 0, 0);

  si5351_set_freq(28074000ULL * 100ULL, SI5351_CLK0); // RX clock, 10m band
  // si5351_set_freq(792000ULL * 100ULL, SI5351_CLK0); // RX clock, 10m band

  return 0;
}
