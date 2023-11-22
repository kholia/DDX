// In search of a FOSS replacement of Icom's RS-BA1 protocol

#include <Wire.h>
#include "pico/stdlib.h"
#include "Arduino.h"

#include <ADCInput.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "CircularBuffer.h"

#define STASSID "XXX"
#define STAPSK "YYY"

WiFiUDP Udp;

#define AVG_BIAS_SHIFT 1u
#define DAC_RANGE  4096u
#define DAC_BIAS  (DAC_RANGE/2u)
#define ADC_RANGE  4096u
#define ADC_BIAS  (ADC_RANGE/2u)

#define ADC_BIAS_SHIFT  20

// const int sampleRate = 16000; // for ADC
const int sampleRate = 44100; // for "sine wave"

// Create input on ADC 0 - GPIO26
ADCInput adcIn(26);

IPAddress ip(192, 168, 68, 107);

#define N 700

CircularBuffer<int16_t, N> buffer;

#define TABLE_SIZE_441HZ            100

// change to 1 for mono output
#define NUM_CHANNELS 1

static int sine_phase;

static const int16_t sine_int16[] = {
  0,    2057,    4107,    6140,    8149,   10126,   12062,   13952,   15786,   17557,
  19260,   20886,   22431,   23886,   25247,   26509,   27666,   28714,   29648,   30466,
  31163,   31738,   32187,   32509,   32702,   32767,   32702,   32509,   32187,   31738,
  31163,   30466,   29648,   28714,   27666,   26509,   25247,   23886,   22431,   20886,
  19260,   17557,   15786,   13952,   12062,   10126,    8149,    6140,    4107,    2057,
  0,   -2057,   -4107,   -6140,   -8149,  -10126,  -12062,  -13952,  -15786,  -17557,
  -19260,  -20886,  -22431,  -23886,  -25247,  -26509,  -27666,  -28714,  -29648,  -30466,
  -31163,  -31738,  -32187,  -32509,  -32702,  -32767,  -32702,  -32509,  -32187,  -31738,
  -31163,  -30466,  -29648,  -28714,  -27666,  -26509,  -25247,  -23886,  -22431,  -20886,
  -19260,  -17557,  -15786,  -13952,  -12062,  -10126,   -8149,   -6140,   -4107,   -2057,
};

// continuous loop running for audio processing
void audioIO_loop(void)
{
  int16_t newSample = 0;

  while (adcIn.available() > 0) {
    newSample = adcIn.read();
    // newSample = newSample - ADC_BIAS - ADC_BIAS_SHIFT;
    // buffer.push(newSample);

    buffer.push(sine_int16[sine_phase]);
    sine_phase++;
    if (sine_phase >= TABLE_SIZE_441HZ) {
      sine_phase -= TABLE_SIZE_441HZ;
    }
  }
}

void audioIO_setup() {
  // https://arduino-pico.readthedocs.io/en/latest/adc.html
  adcIn.setBuffers(4, 32);
  adcIn.onReceive(audioIO_loop);

  if (!adcIn.begin(sampleRate)) {
    Serial.println("Failed to initialize ADCInput!");
    digitalWrite(LED_BUILTIN, 1);
    while (1); // do nothing
  }

  sleep_ms(2000);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  // Setup I/O pins
  Serial.begin(115200);

  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }

  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  Udp.begin(12345);

  audioIO_setup();
}

uint8_t packet[N * 2];

void loop() {
  // audioIO_loop();
  // Serial.println("Alive...");
  if (buffer.isFull()) {
    int n = 0;
    // repeats until the buffer is empty
    while (!buffer.isEmpty()) {
      n = n + 1;
      packet[n++] = buffer.pop();
    }

    Udp.beginPacket(ip, 12345);
    Udp.write(packet, n);
    Udp.endPacket();
  }
}
