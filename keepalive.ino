/*
SUPERMINI
----------
Flash: 4MB
PSRAM: QSPI
Partition: Default 4MB with SPIFFS

N16R8
------
Flash: 16MB
PSRAM: OPI
Partition: 16MB Flash (3MB APP/9.9MB FATFS)
*/


#ifndef ARDUINO_USB_MODE
#error This ESP32 SoC has no Native USB interface
#elif ARDUINO_USB_MODE == 1
#error This sketch should be used when USB is in OTG mode
void setup() {}
void loop() {}
#endif


#include "USB.h"
#include "USBHIDMouse.h"

USBHIDMouse Mouse;


#define BUTTON_PIN 0
#define BUTTON_DEBOUNCE_MS 50  /* typical debounce time for tact button */

#define LED_FLASH_MS 250UL

#define MOVE_INTERVAL_MS 30000UL  /* use 3000UL for testing */
#define MOVE_BACK_DELAY_MS 100UL
#define MOVE_X 1  /* use 100 for testing */


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
    setRgb(0, 0, 4);
    return;
  }

  blueFlashUntil = 0;

  if (enabled) {
    setRgb(0, 2, 0);   // active: green
  } else {
    setRgb(2, 0, 0);   // inactive: red
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
  
  rgbLedWrite(RGB_BUILTIN, 2, 2, 2);  // white: start

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

