// Keep "USB CDC" option disabled.

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>

#include <si5351.h>
#include <ArduinoJson.h>
#include "Adafruit_NeoPixel.h"

Si5351 si5351;

// Your WiFi details
const char* ssid = "field";
const char* password = "password";

WebServer server(80);

// LEF stuff
#ifdef RGB_BRIGHTNESS
#undef RGB_BRIGHTNESS
#endif
#define RGB_BRIGHTNESS 10
#define RGB_BRIGHTNESS 10  // Change white brightness (max 255)
#ifdef RGB_BUILTIN
#undef RGB_BUILTIN
#endif
#define RGB_BUILTIN 10

// GPIO defs
#define PTT 7

// FT8 libs
#include "message.h"
#include "encode.h"
#include "constants.h"
int symbolCount = 79;
int toneSpacing = 625;
#define FT8_TONE_SPACING 625  // ~6.25 Hz
#define FT8_DELAY 159         // Delay value for FT8
#define FT8_SYMBOL_COUNT 79
uint8_t tones[FT8_SYMBOL_COUNT];
int toneDelay = 159;
char current_message[32];
bool message_available = false;
int offset = 1234;

uint64_t freq = 28074000;  // Start on the 10m band by default

void setup() {
  // No need to initialize the RGB LED
  int ret;

  Serial.begin(115200);
  delay(100);

  // I2C pins
  Wire.begin(7, 8);  // ESP32-C3-Zero settings, GP10 has LED!

  // Initialize the Si5351
  ret = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 26000000, 0);  // Dhiru's Si5351 TCXO board
  if (ret != true) {
    // led_flash();
    Serial.println("Si5351 not found!");
    // watchdog_reboot(0, 0, 1000);
  }
  si5351.output_enable(SI5351_CLK0, 0);             // Safety first, TX power off
  si5351.set_freq(28074000 * 100ULL, SI5351_CLK1);  // RX on

  Serial.println("Connecting to ");
  Serial.println(ssid);

  // connect to your local wi-fi network
  WiFi.begin(ssid, password);

  // check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());
  IPAddress gateway;
  gateway = WiFi.gatewayIP();
  Serial.print("Gateway: ");
  Serial.println(gateway.toString());

  server.on("/", handle_OnConnect);
  server.on("/led_on", handle_led_on);
  server.on("/led_off", handle_led_off);
  server.on("/set_message", handle_set_message);
  server.on("/tx", handle_tx);
  server.on("/identify", handle_identify);
  server.on("/change_band", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

  // Initialize mDNS
  if (!MDNS.begin("ddx")) {  // Set the hostname to "esp32.local"
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started!");

  green_led();
}

int ptt_on = false;
int start_actual_tx = false;

void band_change() {
  si5351.set_freq((freq)*100ULL, SI5351_CLK1);  // CLK1 is RX clock
}

void tx() {
  uint8_t i;

  digitalWrite(PTT, 1);
  delay(12);  // Important delay for relay switching
  si5351.output_enable(SI5351_CLK0, 1);

  // TODO: Borrow from https://github.com/kholia/Multi-WSPR/blob/a18b276d62a3bbcf193f2c4014b88ec709ef9943/Tightest-WSPR/Tightest-WSPR.ino#L60
  for (i = 0; i < symbolCount; i++) {
    si5351.set_freq(((freq + offset) * 100ULL) + (tones[i] * toneSpacing), SI5351_CLK0);
    delay(toneDelay);
  }

  // Turn off the output
  si5351.output_enable(SI5351_CLK0, 0);
  digitalWrite(PTT, 0);
  si5351.set_freq((freq)*100ULL, SI5351_CLK1);  // CLK1 is RX clock, reinit rx_clock_line
}

// RGB is GRB it seems!
void red_led() {
  neopixelWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0);
}
void green_led() {
  neopixelWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0);
}
void blue_led() {
  neopixelWrite(RGB_BUILTIN, 0, 0, RGB_BRIGHTNESS);
}
void off_led() {
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);
}

void loop() {
  server.handleClient();

  if (ptt_on == true && start_actual_tx == true) {
    red_led();
    tx();
    start_actual_tx = false;
    ptt_on = false;
    green_led();
  } else {
  }
}

void handle_OnConnect() {
  String output;
  JsonDocument doc;

  doc["status"] = "OK";
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handle_led_on() {
  green_led();
  server.send(200, "text/html", "LED ON OK");
}

void handle_tx() {
  ptt_on = true;
  start_actual_tx = true;

  server.send(200, "text/plain", "TX ON");
}

void handle_identify() {
  server.send(200, "text/plain", "DDX-4");
}

void handle_set_message() {
  ftx_message_t msg;
  ftx_message_rc_t rc;
  bool is_ft4 = false;
  String message;

  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "message") {
      message = server.arg(i);
      break;
    }
  }
  Serial.println(message);  // debug

  rc = ftx_message_encode(&msg, NULL, message.c_str());
  if (rc != FTX_MESSAGE_RC_OK) {
    blue_led();
  } else {
    green_led();
  }
  // Second, encode the binary message as a sequence of FSK tones
  if (is_ft4) {
    ft4_encode(msg.payload, tones);
  } else {
    ft8_encode(msg.payload, tones);
  }

  server.send(200, "text/plain", message);
}

void handle_led_off() {
  off_led();
  server.send(200, "text/plain", "LED OFF OK");
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}
