// Combined from various internet sources.
//
// Code from Sandeep Mistry, Earle F. Philhower, III and others.

struct t_mtab {
  char c, pat;
};

struct t_mtab morsetab[] = {
  {'.', 106},
  {',', 115},
  {'?', 76},
  {'/', 41},
  {'A', 6},
  {'B', 17},
  {'C', 21},
  {'D', 9},
  {'E', 2},
  {'F', 20},
  {'G', 11},
  {'H', 16},
  {'I', 4},
  {'J', 30},
  {'K', 13},
  {'L', 18},
  {'M', 7},
  {'N', 5},
  {'O', 15},
  {'P', 22},
  {'Q', 27},
  {'R', 10},
  {'S', 8},
  {'T', 3},
  {'U', 12},
  {'V', 24},
  {'W', 14},
  {'X', 25},
  {'Y', 29},
  {'Z', 19},
  {'1', 62},
  {'2', 60},
  {'3', 56},
  {'4', 48},
  {'5', 32},
  {'6', 33},
  {'7', 35},
  {'8', 39},
  {'9', 47},
  {'0', 63}
};

#define N_MORSE  (sizeof(morsetab)/sizeof(morsetab[0]))

#define SPEED  (8)
#define DOTLEN  (1200/SPEED)
#define DASHLEN  (3*(1200/SPEED))

#include "button_debounce.h"
#include "cw_generator.h"

CWGenerator *cwgen;

Debounce debouncer;

#include <I2S.h>

// Create the I2S port using a PIO state machine
I2S i2s(OUTPUT);

#define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX                    2                                       // Driver gets this info from the descriptors - we define it here to use it to setup the descriptors and to do calculations with it below
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX                            1                                       // Driver gets this info from the descriptors - we define it here to use it to setup the descriptors and to do calculations with it below - be aware: for different number of channels you need another descriptor!
#define CFG_TUD_AUDIO_EP_SZ_IN                                        (48 + 1) * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX      // 44 Samples (44.1 kHz) x 2 Bytes/Sample x CFG_TUD_AUDIO_N_CHANNELS_TX Channels - the Windows driver always needs an extra sample per channel of space more, otherwise it complains... found by trial and error
// source: https://github.com/hathach/tinyusb/blob/2eaf99e0aa9c10d62dd8d0a4e765f5941bfeaf98/examples/device/audio_4_channel_mic/src/tusb_config.h
#define SAMPLE_RATE ((CFG_TUD_AUDIO_EP_SZ_IN / 2) - 1) * 1000
#define SAMPLE_BUFFER_SIZE ((CFG_TUD_AUDIO_EP_SZ_IN/2) - 1)

// GPIO pin numbers for the DAC
#define pBCLK 15
#define pWS (pBCLK+1)
#define pDOUT 17

const int frequency = 850; // frequency of square wave in Hz
const int amplitude = 8000; // amplitude of square wave
const int sampleRate = SAMPLE_RATE; // good for PT8211

const int halfWavelength = (sampleRate / frequency); // half wavelength of square wave

int16_t sample = amplitude; // current sample value
uint64_t count = 0;


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // digitalWrite(LED_BUILTIN, 1);

  Serial.begin(115200);
  Serial.println("I2S simple tone");

  // delay(5000);

  i2s.setBCLK(pBCLK);
  i2s.setDATA(pDOUT);
  i2s.setBitsPerSample(16);
  i2s.setLSBJFormat();

  // start I2S at the sample rate with 16-bits per sample
  if (!i2s.begin(sampleRate)) {
    digitalWrite(LED_BUILTIN, 1);
    Serial.println("Failed to initialize I2S!");
    while (1); // do nothing
  }

  cwgen = new CWGenerator(SAMPLE_RATE, SAMPLE_BUFFER_SIZE);
}

void loop() {
  // check keys, update state, calculate next buffer
  int ret = cwgen->update_statemachine();

  if (ret != 2) { // STATE_IDLE
    int size = cwgen->get_audio_buffer_size();
    uint16_t *buf = (uint16_t *)cwgen->get_audio_buffer();
    for (int i = 0; i < size / 2; i++) {
      i2s.write(buf[i]);
      i2s.write(buf[i]);
    }
  } else {
    // sendmsg((char *)"CQ CQ CQ DE VU3CER");
    // digitalWrite(LED_BUILTIN, 1);
    if (count % halfWavelength == 0) {
      // invert the sample every half wavelength count multiple to generate square wave
      sample = -1 * sample;
    }
    // write the same sample twice, once for left and once for the right channel
    i2s.write(sample);
    i2s.write(sample);

    // increment the counter for the next sample
    count++;
    // digitalWrite(LED_BUILTIN, 0);
  }
}
