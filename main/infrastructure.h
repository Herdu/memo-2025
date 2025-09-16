#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ----------------- Melody -----------------
class Melody
{
private:
    const int *notes;
    const int *durations;
    const int length;

public:
    Melody(const int *n, const int *d, int len);

    int getNote(int index) const;
    int getDuration(int index) const;
    int getLength() const;
};

// ----------------- Buzzer -----------------
class BuzzerController
{
private:
    const uint8_t pin;
    unsigned long lastNoteTime = 0;
    int melodyIndex = 0;
    bool playing = false;
    const Melody *currentMelody = nullptr;

public:
    BuzzerController(uint8_t buzzerPin);

    void setup();
    void playTone(int note = 0, int duration = 200);
    void startMelody(const Melody &melody);
    void update();
    bool isPlaying();
};

// ----------------- LCD -----------------
class LCD
{
private:
    LiquidCrystal_I2C lcd;

public:
    LCD(uint8_t address, uint8_t cols, uint8_t rows);
    void setup();
    void print(const char *line1, const char *line2);
    void print(const String &line1, const String &line2);
};

// ----------------- Button + LED -----------------
class ButtonLed
{
private:
    int pinButton;
    int pinLed;
    const Melody *melody;

public:
    ButtonLed(int button, int led, const Melody *m);

    void setup();
    bool isPressed();
    void ledOn();
    void ledOff();
    void handle();
};
