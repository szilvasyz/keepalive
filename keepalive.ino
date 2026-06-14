/*
Recommended settings for KeepAlive:

ESP32-S3 SuperMini
------------------
Flash: 4MB
PSRAM: Disabled for this project
Partition: Default 4MB with SPIFFS
USB Mode: USB-OTG (TinyUSB)
USB CDC On Boot: Enabled

ESP32-S3 N16R8 dev module
-------------------------
Flash: 4MB or 16MB
PSRAM: Disabled for this project
Partition: Default 4MB is enough
USB Mode: USB-OTG (TinyUSB)
USB CDC On Boot: Enabled

Notes:
- PSRAM is not used by this project.
- If RGB colors are wrong on another board, adjust channel order in setRgb().

N16R8 dev module notes:
- Use the connector labelled "USB"
- Built-in RGB LED may require solder jumper to connect it to the GPIO.
  If the jumper is open, firmware still works but status LED is not visible.

*/


//
// Build-phase Board Checking
//
#ifndef ARDUINO_USB_MODE
#error This ESP32 SoC has no Native USB interface
#elif ARDUINO_USB_MODE == 1
#error This sketch should be used when USB is in OTG mode
void setup() {}
void loop() {}
#endif

//
// Operating Parameters
//
#define BUTTON_PIN 0
#define BUTTON_DEBOUNCE_MS 50UL /* typical debounce time for tact button */

#define LED_FLASH_MS 250UL
#define LED_INTENSITY 8 /* 0 - 255, value for RGB channels */

#define MOVE_INTERVAL_MS 30000UL /* use 3000UL for testing */
#define MOVE_BACK_DELAY_MS 100UL
#define MOVE_X 1 /* use 100 for testing */


//
// Main Code
//
#include "USB.h"
#include "USBHIDMouse.h"

USBHIDMouse Mouse;


bool enabled = false;

bool buttonStable = HIGH;
bool buttonLastRead = HIGH;
uint32_t buttonChangedAt = 0;

uint32_t lastMoveAt = 0;
uint32_t blueFlashUntil = 0;

void setRgb(uint8_t r, uint8_t g, uint8_t b) {
  rgbLedWrite(RGB_BUILTIN, r, g, b);
}

void updateLed() {
  uint32_t now = millis();

  if (blueFlashUntil && now < blueFlashUntil) {
    setRgb(0, 0, LED_INTENSITY);
    return;
  }

  blueFlashUntil = 0;

  if (enabled) {
    setRgb(0, LED_INTENSITY, 0);  // active: green
  } else {
    setRgb(LED_INTENSITY, 0, 0);  // inactive: red
  }
}

void updateButton() {
  uint32_t now = millis();
  bool raw = digitalRead(BUTTON_PIN);

  if (raw != buttonLastRead) {
    buttonLastRead = raw;
    buttonChangedAt = now;
  }

  if ((now - buttonChangedAt) >= (BUTTON_DEBOUNCE_MS) && raw != buttonStable) {
    buttonStable = raw;

    if (buttonStable == LOW) {
      enabled = !enabled;
      lastMoveAt = now;
      Serial.printf("Enabled: %s\n", enabled ? "ON" : "OFF");
    }
  }
}

void updateKeepAlive() {
  if (!enabled) return;

  uint32_t now = millis();

  if (now - lastMoveAt >= MOVE_INTERVAL_MS) {
    lastMoveAt = now;

    Mouse.move(MOVE_X, 0, 0);
    delay(MOVE_BACK_DELAY_MS);
    Mouse.move(-(MOVE_X), 0, 0);

    blueFlashUntil = now + LED_FLASH_MS;
    Serial.println("Move");
  }
}


void setup() {

  setRgb(LED_INTENSITY, LED_INTENSITY, LED_INTENSITY);  // white: start

  delay(3000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Mouse.begin();
  USB.begin();
  Serial.begin(115200);

  Serial.println("Hello ESP32S3 KeepAlive");
  Serial.printf("PSRAM size: %u\n", ESP.getPsramSize());
  Serial.printf("PSRAM free: %u\n", ESP.getFreePsram());

  updateLed();
}

void loop() {

  updateButton();
  updateKeepAlive();
  updateLed();
}
