#include <si5351.h>
#include "Wire.h"
#include <EEPROM.h>

#include "SI4735.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"
#include <stdio.h>
#include "hardware/pwm.h"
#include "pico/multicore.h"
#include "hardware/adc.h"
#include "hardware/uart.h"

Si5351 si5351;

#define RESET_PIN 16

int cal_factor = 0;

// Debug helper
void led_flash()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
}

#define CAL            15 // Automatic calibration entry

// Automatic calibration
uint32_t f_hi_;

void pwm_int_() {
  pwm_clear_irq(7);
  f_hi_++;
}

#include "Rotary.h"
// Test it with patch_init.h or patch_full.h. Do not try load both.
// #include <patch_init.h> // SSB patch for whole SSBRX initialization string
#include "patch_full.h"    // SSB patch for whole SSBRX full download

const uint16_t size_content = sizeof ssb_patch_content; // see ssb_patch_content in patch_full.h or patch_init.h

#define AM_FUNCTION 1

#define LSB 1
#define USB 2

bool disableAgc = true;
bool avc_en = true;

int currentBFO = 0;

// Some variables to check the SI4735 status
uint16_t currentFrequency;
uint8_t currentStep = 1;
uint8_t currentBFOStep = 25;

uint8_t bandwidthIdx = 2;
const char *bandwidth[] = {"1.2", "2.2", "3.0", "4.0", "0.5", "1.0"};
long et1 = 0, et2 = 0;

typedef struct
{
  uint16_t minimumFreq;
  uint16_t maximumFreq;
  uint16_t currentFreq;
  uint16_t currentStep;
  uint8_t currentSSB;
} Band;

Band band[] = {
  {14000, 14300, 14074, 1, USB},
  {21000, 21300, 21074, 1, USB},
  {28000, 28300, 28124, 1, USB},
};

const int lastBand = (sizeof band / sizeof(Band)) - 1;
int currentFreqIdx = 2;
uint8_t currentAGCAtt = 0;

uint8_t rssi = 0;

SI4735 si4735;

void setup()
{
  int ret;

  pinMode(CAL, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  // delay(7000);

  // I2C pins
  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire.begin();

  // Initialize the Si5351
  ret = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 26000000, 0); // Dhiru's Si5351 TCXO board
  if (ret != true) {
    led_flash();
    watchdog_reboot(0, 0, 1000);
  }

  si5351.output_enable(SI5351_CLK0, 0); // Safety first
  si5351.set_freq(3276800, SI5351_CLK0);
  si5351.output_enable(SI5351_CLK0, 1);

  // gets and sets the Si47XX I2C bus address.
  int16_t si4735Addr = si4735.getDeviceI2CAddress(RESET_PIN);
  if ( si4735Addr == 0 ) {
    Serial.println("Si473X not found!");
    Serial.flush();
    while (1);
  } else {
    Serial.print("The Si473X I2C address is 0x");
    Serial.println(si4735Addr, HEX);
  }

  si4735.setup(RESET_PIN, AM_FUNCTION);

  // Testing I2C clock speed and SSB behaviour
  // si4735.setI2CLowSpeedMode();     //  10000 (10kHz)
  // si4735.setI2CStandardMode();     // 100000 (100kHz)
  // si4735.setI2CFastMode();         // 400000 (400kHz)
  // si4735.setI2CFastModeCustom(500000); // -> It is not safe and can crash.
  delay(10);
  Serial.println("SSB patch is loading...");
  et1 = millis();
  loadSSB();
  et2 = millis();
  Serial.print("SSB patch was loaded in: ");
  Serial.print( (et2 - et1) );
  Serial.println("ms");
  delay(100);
  si4735.setTuneFrequencyAntennaCapacitor(1); // Set antenna tuning capacitor for SW.
  si4735.setSSB(band[currentFreqIdx].minimumFreq, band[currentFreqIdx].maximumFreq, band[currentFreqIdx].currentFreq, band[currentFreqIdx].currentStep, band[currentFreqIdx].currentSSB);
  delay(100);
  currentFrequency = si4735.getFrequency();
  si4735.setAvcAmMaxGain(90); // Sets the maximum gain for automatic volume control on AM/SSB mode (from 12 to 90dB)
  si4735.setVolume(63); // 63 is max
  si4735.setSSBBfo(-800);
  showHelp();
  showStatus();
}

void showSeparator()
{
  Serial.println("\n**************************");
}

void showHelp()
{
  showSeparator();
  Serial.println("Type: ");
  Serial.println("U to frequency up or D to frequency down");
  Serial.println("> to go to the next band or < to go to the previous band");
  Serial.println("W to switch the filter bandwidth");
  Serial.println("B to go to increment the BFO or b decrement the BFO");
  Serial.println("G to switch on/off the Automatic Gain Control");
  Serial.println("A to switch the LNA Gain Index (0, 1, 5, 15 e 26");
  Serial.println("S to switch the frequency increment and decrement step");
  Serial.println("s to switch the BFO increment and decrement step");
  Serial.println("X Shows the current status");
  Serial.println("H to show this help");
}

// Show current frequency
void showFrequency()
{
  String freqDisplay;
  freqDisplay = String((float)currentFrequency / 1000, 3);
  showSeparator();
  Serial.print("Current Frequency: ");
  Serial.print(freqDisplay);
  Serial.print("kHz");
  Serial.print(" | Step: ");
  Serial.println(currentStep);
}

void showStatus()
{
  showSeparator();
  Serial.print("SSB | ");

  si4735.getAutomaticGainControl();
  si4735.getCurrentReceivedSignalQuality();

  Serial.print((si4735.isAgcEnabled()) ? "AGC ON " : "AGC OFF");
  Serial.print(" | LNA GAIN index: ");
  Serial.print(si4735.getAgcGainIndex());
  Serial.print("/");
  Serial.print(currentAGCAtt);

  Serial.print(" | BW :");
  Serial.print(String(bandwidth[bandwidthIdx]));
  Serial.print("kHz");
  Serial.print(" | SNR: ");
  Serial.print(si4735.getCurrentSNR());
  Serial.print(" | RSSI: ");
  Serial.print(si4735.getCurrentRSSI());
  Serial.print(" dBuV");
  Serial.print(" | Volume: ");
  Serial.println(si4735.getVolume());
  showFrequency();
}

void showBFO()
{
  String bfo;

  if (currentBFO > 0)
    bfo = "+" + String(currentBFO);
  else
    bfo = String(currentBFO);

  showSeparator();

  Serial.print("BFO: ");
  Serial.print(bfo);
  Serial.print("Hz ");

  Serial.print(" | Step: ");
  Serial.print(currentBFOStep);
  Serial.print("Hz ");
}

void bandUp()
{
  // save the current frequency for the band
  band[currentFreqIdx].currentFreq = currentFrequency;
  if (currentFreqIdx < lastBand)
  {
    currentFreqIdx++;
  }
  else
  {
    currentFreqIdx = 0;
  }
  si4735.setTuneFrequencyAntennaCapacitor(1); // Set antenna tuning capacitor for SW.
  si4735.setSSB(band[currentFreqIdx].minimumFreq, band[currentFreqIdx].maximumFreq, band[currentFreqIdx].currentFreq, band[currentFreqIdx].currentStep, band[currentFreqIdx].currentSSB);
  currentStep = band[currentFreqIdx].currentStep;
  delay(250);
  currentFrequency = si4735.getCurrentFrequency();
  showStatus();
}

void bandDown()
{
  // save the current frequency for the band
  band[currentFreqIdx].currentFreq = currentFrequency;
  if (currentFreqIdx > 0)
  {
    currentFreqIdx--;
  }
  else
  {
    currentFreqIdx = lastBand;
  }
  si4735.setTuneFrequencyAntennaCapacitor(1); // Set antenna tuning capacitor for SW.
  si4735.setSSB(band[currentFreqIdx].minimumFreq, band[currentFreqIdx].maximumFreq, band[currentFreqIdx].currentFreq, band[currentFreqIdx].currentStep, band[currentFreqIdx].currentSSB);
  currentStep = band[currentFreqIdx].currentStep;
  delay(250);
  currentFrequency = si4735.getCurrentFrequency();
  showStatus();
}

/*
   This function loads the contents of the ssb_patch_content array into the CI (Si4735) and starts the radio on
   SSB mode.
*/
void loadSSB()
{
  si4735.setI2CFastModeCustom(500000); // Increase the transfer I2C speed
  si4735.loadPatch(ssb_patch_content, size_content); // It is a legacy function. See loadCompressedPatch
  // Parameters
  // AUDIOBW - SSB Audio bandwidth; 0 = 1.2kHz (default); 1=2.2kHz; 2=3kHz; 3=4kHz; 4=500Hz; 5=1kHz;
  // SBCUTFLT SSB - side band cutoff filter for band passand low pass filter ( 0 or 1)
  // AVC_DIVIDER  - set 0 for SSB mode; set 3 for SYNC mode.
  // AVCEN - SSB Automatic Volume Control (AVC) enable; 0=disable; 1=enable (default).
  // SMUTESEL - SSB Soft-mute Based on RSSI or SNR (0 or 1).
  // DSP_AFCDIS - DSP AFC Disable or enable; 0=SYNC MODE, AFC enable; 1=SSB MODE, AFC disable.
  si4735.setSSBConfig(bandwidthIdx, 1, 0, 1, 0, 1);
  si4735.setI2CFastModeCustom(100000); // Set standard transfer I2C speed
}

/*
   Main
*/
void loop()
{
  // Check if exist some command to execute
  if (Serial.available() > 0)
  {
    char key = Serial.read();
    if (key == 'U' || key == 'u')
    { // frequency up
      si4735.frequencyUp();
      delay(250);
      band[currentFreqIdx].currentFreq = currentFrequency = si4735.getCurrentFrequency();
      showFrequency();
    }
    else if (key == 'D' || key == 'd')
    { // frequency down
      si4735.frequencyDown();
      delay(250);
      band[currentFreqIdx].currentFreq = currentFrequency = si4735.getCurrentFrequency();
      showFrequency();
    }
    else if (key == 'W' || key == 'w') // sitches the filter bandwidth
    {
      bandwidthIdx++;
      if (bandwidthIdx > 5)
        bandwidthIdx = 0;
      si4735.setSSBAudioBandwidth(bandwidthIdx);
      // If audio bandwidth selected is about 2 kHz or below, it is recommended to set Sideband Cutoff Filter to 0.
      if (bandwidthIdx == 0 || bandwidthIdx == 4 || bandwidthIdx == 5)
        si4735.setSSBSidebandCutoffFilter(0);
      else
        si4735.setSSBSidebandCutoffFilter(1);
      showStatus();
    }
    else if (key == '>' || key == '.') // goes to the next band
      bandUp();
    else if (key == '<' || key == ',') // goes to the previous band
      bandDown();
    else if (key == 'V')
    { // volume down
      si4735.volumeUp();
      showStatus();
    }
    else if (key == 'v')
    { // volume down
      si4735.volumeDown();
      showStatus();
    }
    else if (key == 'B') // increments the bfo
    {
      currentBFO += currentBFOStep;
      si4735.setSSBBfo(currentBFO);
      showBFO();
    }
    else if (key == 'b') // decrements the bfo
    {
      currentBFO -= currentBFOStep;
      si4735.setSSBBfo(currentBFO);
      showBFO();
    }
    else if (key == 'G' || key == 'g') // switches on/off the Automatic Gain Control
    {
      disableAgc = !disableAgc;
      // siwtch on/off ACG; AGC Index = 0. It means Minimum attenuation (max gain)
      si4735.setAutomaticGainControl(disableAgc, currentAGCAtt);
      showStatus();
    }
    else if (key == 'A' || key == 'a') // Switches the LNA Gain index  attenuation
    {
      if (currentAGCAtt == 0)
        currentAGCAtt = 1;
      else if (currentAGCAtt == 1)
        currentAGCAtt = 5;
      else if (currentAGCAtt == 5)
        currentAGCAtt = 15;
      else if (currentAGCAtt == 15)
        currentAGCAtt = 26;
      else
        currentAGCAtt = 0;
      si4735.setAutomaticGainControl(1 /*disableAgc*/, currentAGCAtt);
      showStatus();
    }
    else if (key == 's') // switches the BFO increment and decrement step
    {
      currentBFOStep = (currentBFOStep == 50) ? 10 : 50;
      showBFO();
    }
    else if (key == 'S') // switches the frequency increment and decrement step
    {
      if (currentStep == 1)
        currentStep = 5;
      else if (currentStep == 5)
        currentStep = 10;
      else
        currentStep = 1;
      si4735.setFrequencyStep(currentStep);
      band[currentFreqIdx].currentStep = currentStep;
      showFrequency();
    }
    else if (key == 'C' || key == 'c') // switches on/off the Automatic Volume Control
    {
      avc_en = !avc_en;
      si4735.setSSBAutomaticVolumeControl(avc_en);
    }
    else if (key == 'X' || key == 'x')
      showStatus();
    else if (key == 'H' || key == 'h')
      showHelp();
  }
  delay(200);
}
