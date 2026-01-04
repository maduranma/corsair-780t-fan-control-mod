#include "BulkUSB.h"

const int ledPins[3] = {5, 6, 3};
const int buttonPin = 4;

void setup() {
    for (int i = 0; i < 3; i++) {
        pinMode(ledPins[i], OUTPUT);
        analogWrite(ledPins[i], 0);
    }
    pinMode(buttonPin, INPUT_PULLUP);
}

int oldButton = 0;
void loop() {
  char buf[2];
  int n = BulkUSB.read(buf, sizeof(buf));
  if (n == 2) {
    int led = buf[0];
    int value = buf[1];
    if(value == -2) {
      enableBlink(led);
      return;
    }

    disableBlink(led);
    analogWrite(ledPins[led], 255 - (int)((value * 255UL) / 100UL));
  }

  int currentButton = digitalRead(buttonPin);
  if(currentButton != oldButton) {
    oldButton = currentButton;
    uint8_t out = 1 - currentButton;
    BulkUSB.write(&out, 1);
    delay(20);
  }

  doBlinking();
  delay(1);
}



#define INC 0
#define DEC 1
int blinkingDirection = INC;
uint8_t blinkingBrightness = 0;
#define BLINKING_DELAY 3
int currentBlinkingDelay = 0;

bool ledBlinking[3] = {true, true, true};

void disableBlink(int led) {
  ledBlinking[led] = false;
}

void enableBlink(int led) {
  bool anyLedBlinking = false;
  for(int i = 0; i < 3; i++) {
    anyLedBlinking = ledBlinking[i];
  }
  if (!anyLedBlinking) {
    blinkingBrightness = 0;
    blinkingDirection = INC;
  }
  ledBlinking[led] = true;
}

void doBlinking() {
  if(currentBlinkingDelay++ != BLINKING_DELAY) {
    return;
  }
  currentBlinkingDelay = 0;

  if(blinkingDirection == INC) {
    if(++blinkingBrightness == 0) {
      blinkingBrightness = 255;
      blinkingDirection = DEC;
    }
  } else if(blinkingDirection == DEC) {
    if(--blinkingBrightness == 255) {
      blinkingBrightness = 0;
      blinkingDirection = INC;
    }
  }

  for(int i = 0; i < 3; i++) {
    if(ledBlinking[i]) {
      analogWrite(ledPins[i], 255 - blinkingBrightness);
    }
  }
}