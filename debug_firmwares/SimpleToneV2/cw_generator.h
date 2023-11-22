/*
   The MIT License (MIT)

   Copyright (c) 2022 Jochen Schaeuble

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

*/

#ifndef _CW_GENERATOR_H_
#define _CW_GENERATOR_H_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "hardware/pio.h"
#include "button_debounce.h"

/*
   class that generates and audio buffer that contains morse code signals.
*/

#define DIT_GPIO 3                  // GPIO port for the DIT paddle
#define DIT_UNITS 1                 // number of time units for a DIT
#define DAH_GPIO 4                  // GPIO port for the DAH paddle
#define DAH_UNITS 3                 // number of time units for a DAH
#define INTRA_CHAR_PAUSE_UNITS 1    // number of time units for a pause within a characters
#define INTER_CHAR_PAUSE_UNITS 3    // number of time units for a pause between characters
#define INT_WORD_PAUSE_UNITS 7      // number of time units for a pause between words

#define DEFAULT_FREQUENCY 700       // default frequency for the audio tone
#define DEFAULT_WPM 10              // default speed for the morse code in WPM (Words Per Minute) // NOTE!
#define DEFAULT_VOLUME 100          // default volume [%] of the morse signal
#define DEFAULT_RISETIME 10         // default risetime of the Blackman window

#define MAX_VOLUME 32000            // maximum volume (32768 * 0.75) - 1

#define WPM_MIN 10                  // minimum speed in WPM
#define WPM_MAX 99                  // maximum speed in WPM

#define RISETIME_MIN 1              // minimum risetime of the Blackman window
#define RISETIME_MAX 100            // maximum risetime of the Blackman window

#define LPF_HALFORDER 4/2           // order / 2 of the Butterworth low pass filter

class CWGenerator
{
  public:
    const static uint16_t audio_minfreq = 400;  // minimum audio frequency for the morse code signal
    const static uint16_t audio_maxfreq = 850;  // maximum audio frequency for the morse code signal
    const static uint16_t queue_max_char = 255; // maximum number of characters that can be stored in the queue

    // Possible morse code characters
    typedef enum {
      CHAR_PAUSE,
      CHAR_DIT,
      CHAR_DAH
    } CW_CHARACTERS;

    // Different states of the morse code state machine
    typedef enum {
      STATE_INIT,
      STATE_INIT_PAUSE,
      STATE_IDLE,
      STATE_DIT,
      STATE_DIT_PAUSE,
      STATE_DAH,
      STATE_DAH_PAUSE
    } CW_STATE;

    /*
       constructor for the morse code sound generator with default frequency and speed
       @param sample_rate: sample rate of the audio signal
       @param sample_buffer_size: size of the buffer used to transmit the audio signal
    */
    CWGenerator(uint32_t sample_rate, uint32_t sample_buffer_size);

    /*
       constructor for the morse code sound generator
       @param sample_rate: sample rate of the audio signal
       @param sample_buffer_size: size of the buffer used to transmit the audio signal
       @param freq: frequency of the audio signal
       @param wpm: speed of the morse code in WPM (Words Per Minute)
       @param volume: volume of the signal [0:100]
       @param risetime: rise time of the Blackman window
    */
    CWGenerator(uint32_t sample_rate, uint32_t sample_buffer_size, uint16_t freq, uint16_t wpm, uint16_t volume, float risetime);

    /*
       set the audio frequency in Hz of the sine wave
       @param freq: frequency of the audio signal.
                    the value must be between [audio_minfreq, audio_maxfreq]
    */
    void set_frequency(uint16_t freq);

    /*
       get the audio frequency in Hz of the sine wave
       @return frequency of the audio signal.
    */
    uint16_t get_frequency();

    /*
       set the speed auf the morse signal in WPM (Words Per Minute)
       @param wpm: the speed in WPM
    */
    void set_wpm(uint16_t wpm);

    /*
       get the speed auf the morse signal in WPM (Words Per Minute)
       @return the speed in WPM
    */
    uint16_t get_wpm();

    /*
       set the rise time of the Blackman window
       @param wpm: rise time in ms
    */
    void set_risetime(float risetime);

    /*
       get the speed auf the morse signal in WPM (Words Per Minute)
       @return rise time in ms
    */
    float get_risetime();

    /*
       set the volume of the morse signal [0:100]
       @param volume: volume [%] of the morse signal
    */
    void set_volume(uint16_t vol);

    /*
       get the volume of the morse signal
       @return volume of the morse signal
    */
    uint16_t get_volume();

    /*
       adds a morse code character to the transmission queue
       @param ch: character to be send out
    */
    void send_character(CW_CHARACTERS ch);

    /*
      adds a morse code character to the transmission queue
      @param ch: character to be send out (' ' -> Pause, '.' -> DIT, '-' -> DAH)
    */
    void send_character(char *ch);

    /*
       Updates the state machine and checks the paddle position
    */
    int update_statemachine();

    /*
       Returns the audio buffer for the next transmission
       @return buffer consisting of an array of int16_t samples
    */
    void *get_audio_buffer();

    /*
       Returns the audio buffer size for the next transmission
       @return buffer size in uint32_t
    */
    uint32_t get_audio_buffer_size();

  private:
    uint32_t cw_sample_rate;                    // sample rate of the audio signal
    uint32_t cw_sample_buffer_size;             // size of the sample buffer used to transmit the audio signal
    uint8_t cw_wpm;                             // CW speed in WPM
    uint16_t cw_frequency;                      // tone frequency in Hz
    uint16_t cw_volume;                         // volume of the audio signal [0:32767]
    float cw_risetime;                          // rise time of keyed signal in ms
    uint32_t cw_risetime_samples;               // nr. of samples for the rise time
    uint32_t cw_risetime_samples_maxsize;       // maximum number of samples for rise time
    float *cw_keyshape;                         // buffer containing the key shape factors of the Blackman window
    uint32_t cw_keyshape_stepsize;              // step size between samples in keyshape table

    float *signal_buffer;                     // buffer containing a single sine wave
    int16_t *output_buffer;                     // buffer used to tramsmit the audio to the USB port
    uint32_t signal_buffer_period;              // sine wave period
    uint32_t signal_dit_length_index;           // number of samples for a DIT in the current CW speed

    queue_t cw_character_queue;                 // used to send characters to the morse code state machine
    Debounce debouncer;                         // Debouncer used for the paddle input

    CW_CHARACTERS curchar;
    CW_STATE curstate;                          // current state of the state machine
    CW_STATE nextstate;                         // next state after the current state is finished

    uint32_t inchar_index;                      // sound buffer index within the current morse character
    uint32_t inchar_endindex;                   // end index in number of tone_buffer_periods

    PIO ws2812_pio;                             // PIO used for the Neopixel LED
    int ws2812_sm;                              // PIO statemachine for Neopixel LED

    /*
       initializes the audio buffers for the currently set frequency
    */
    void init_buffers();

    /*
       initializes the Butterworth low pass filter
    */
    void init_filter();

    /*
       clears the character queue
    */
    void clear_queue();

    /*
       helper function to set a new state of the CW state machine
       @param ch: character to be send out
       @param ws2812_color: color of the Neopixel LED
    */
    void set_state(CW_CHARACTERS ch);

    /*
       set the integrated Neopixel to the specified color
       @param pixel_grb: color of the Neopixel LED (r << 8 | g << 16 | b)
    */
    inline void put_pixel(uint32_t pixel_grb);
};

#endif
