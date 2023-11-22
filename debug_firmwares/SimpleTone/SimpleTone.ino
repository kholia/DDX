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


#include <I2S.h>

// Create the I2S port using a PIO state machine
I2S i2s(OUTPUT);

// GPIO pin numbers
#define pBCLK 15
#define pWS (pBCLK+1)
#define pDOUT 17

const int frequency = 850; // frequency of square wave in Hz
const int amplitude = 4000; // amplitude of square wave
const int sampleRate = 16000; // good for PT8211

const int halfWavelength = (sampleRate / frequency); // half wavelength of square wave

int16_t sample = amplitude; // current sample value
uint64_t count = 0;


void playDith() {
  playTone(1);
}

void playDash() {
  playTone(4);
}

void playDelay() {
  delay(100);
}


void playTone(int mult) {
  int16_t sample = amplitude; // current sample value
  int count = 0;

  for (int i = 1; i <= (360 * mult); i++) {

    if (count % halfWavelength == 0) {
      // invert the sample every half wavelength count multiple to generate square wave
      sample = -1 * sample;
    }

    // write the same sample twice, once for left and once for the right channel
    i2s.write(sample);
    i2s.write(sample);

    // increment the counter for the next sample
    count++;

  }
  delay(85);
}

void playTest() {
}

void playCW() {
  // C
  playDash();
  playDith();
  playDash();
  playDith();

  playDelay();

  // W
  playDith();
  playDash();
  playDash();
  playDelay();
}

void playCN() {  // C
  playDash();
  playDith();
  playDash();
  playDith();

  playDelay();

  // N

  playDash();
  playDith();
  playDelay();
}

void playSW() {
  // S
  playDith();
  playDith();
  playDith();

  playDelay();

  // S
  playDith();
  playDith();
  playDith();

  playDelay();


  // W
  playDith();
  playDash();
  playDash();
  playDelay();
}

void playSN() {
  // S
  playDith();
  playDith();
  playDith();

  playDelay();

  // S
  playDith();
  playDith();
  playDith();

  playDelay();

  // N

  playDash();
  playDith();
  playDelay();
}

void playNN() {
  // N

  playDash();
  playDith();
  playDelay();

  playDelay();

  // N

  playDash();
  playDith();
  playDelay();
}


void playN1() {
  // N

  playDash();
  playDith();

  playDelay();

  // 1
  playDith();
  playDash();
  playDash();
  playDash();
  playDash();

  playDelay();
}


void playN2() {
  // N

  playDash();
  playDith();
  playDelay();

  // 2
  playDith();
  playDith();
  playDash();
  playDash();
  playDash();

  playDelay();
}

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
}

void send(char c) {
  unsigned int i;
  if (c == ' ') {
    delay(7 * DOTLEN);
    return;
  }
  for (i = 0; i < N_MORSE; i++) {
    if (morsetab[i].c == c) {
      unsigned char p = morsetab[i].pat;
      Serial.print(morsetab[i].c);

      while (p != 1) {
        if (p & 1)
          playDash();
        else
          playDith();
        p = p / 2;
      }
      playDelay();
      return;
    }
  }
  /* if we drop off the end, then we send a space */
  Serial.print("?");
}


void sendmsg(char *str) {
  while (*str)
    send(*str++);
}

void loop() {
  sendmsg((char *)"CQ CQ CQ DE VU3CER");

  /*
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
    // digitalWrite(LED_BUILTIN, 0); */
}
