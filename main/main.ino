#include "infrastructure.h"

// ----- Melodies -----
int melody1Notes[] = { 262, 294, 330, 349 };
int melody1Durations[] = { 200, 200, 200, 400 };
Melody melody1(melody1Notes, melody1Durations, 4);

int melody2Notes[] = { 523, 494, 440, 392 };
int melody2Durations[] = { 200, 200, 200, 400 };
Melody melody2(melody2Notes, melody2Durations, 4);

int melody3Notes[] = { 330, 392, 440, 523 };
int melody3Durations[] = { 150, 150, 150, 400 };
Melody melody3(melody3Notes, melody3Durations, 4);

int melody4Notes[] = { 440, 0, 440, 0, 440 };
int melody4Durations[] = { 150, 100, 150, 100, 300 };
Melody melody4(melody4Notes, melody4Durations, 5);

// ----- Global Objects -----
BuzzerController buzzerController(2);
LCD myLCD(0x27, 16, 2);

ButtonLed sets[] = {
  ButtonLed(6, 7, &melody1),
  ButtonLed(8, 9, &melody2),
  ButtonLed(10, 11, &melody3),
  ButtonLed(12, 13, &melody4),
};
int count = 4;

void setup() {
  for (int i = 0; i < count; i++) {
    sets[i].setup();
  }
  myLCD.setup();
  myLCD.print("Playing:", "Twinkle");
  buzzerController.setup();
}

void loop() {
  buzzerController.update();
  for (int i = 0; i < count; i++) {
    sets[i].handle();
    if (sets[i].isPressed()) {
      myLCD.print("Button pressed:", "Button " + String(i + 1));
    }
  }
}