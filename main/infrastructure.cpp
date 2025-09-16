#include "infrastructure.h"

// ----------------- Melody -----------------
Melody::Melody(const int *n, const int *d, int len)
    : notes(n), durations(d), length(len) {}

int Melody::getNote(int index) const { return notes[index]; }
int Melody::getDuration(int index) const { return durations[index]; }
int Melody::getLength() const { return length; }

// ----------------- Buzzer -----------------
BuzzerController::BuzzerController(uint8_t buzzerPin) : pin(buzzerPin) {}

void BuzzerController::setup()
{
    pinMode(pin, OUTPUT);
}

void BuzzerController::playTone(int note, int duration)
{
    if (note > 0)
    {
        tone(pin, note, duration);
    }
    else
    {
        noTone(pin);
    }
}

void BuzzerController::startMelody(const Melody &melody)
{
    currentMelody = &melody;
    melodyIndex = 0;
    lastNoteTime = millis();
    playing = true;
    playTone(currentMelody->getNote(melodyIndex),
             currentMelody->getDuration(melodyIndex));
}

void BuzzerController::update()
{
    if (playing && currentMelody != nullptr)
    {
        unsigned long currentTime = millis();
        int duration = currentMelody->getDuration(melodyIndex);

        if (currentTime - lastNoteTime >= (unsigned long)duration)
        {
            melodyIndex++;
            if (melodyIndex < currentMelody->getLength())
            {
                lastNoteTime = currentTime;
                playTone(currentMelody->getNote(melodyIndex),
                         currentMelody->getDuration(melodyIndex));
            }
            else
            {
                noTone(pin);
                playing = false;
                currentMelody = nullptr;
            }
        }
    }
}

bool BuzzerController::isPlaying() { return playing; }

// ----------------- LCD -----------------
LCD::LCD(uint8_t address, uint8_t cols, uint8_t rows)
    : lcd(address, cols, rows) {}

void LCD::setup()
{
    lcd.init();
    lcd.backlight();
}

void LCD::print(const char *line1, const char *line2)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}

void LCD::print(const String &line1, const String &line2)
{
    print(line1.c_str(), line2.c_str());
}

// ----------------- Button + LED -----------------
ButtonLed::ButtonLed(int button, int led, const Melody *m)
{
    pinButton = button;
    pinLed = led;
    melody = m;
}

void ButtonLed::setup()
{
    pinMode(pinButton, INPUT_PULLUP);
    pinMode(pinLed, OUTPUT);
    digitalWrite(pinLed, HIGH); // LED off initially
}

bool ButtonLed::isPressed()
{
    return digitalRead(pinButton) == LOW;
}

void ButtonLed::ledOn()
{
    digitalWrite(pinLed, LOW);
}

void ButtonLed::ledOff()
{
    digitalWrite(pinLed, HIGH);
}

void ButtonLed::handle()
{
    extern BuzzerController buzzerController; // declared in main file
    if (isPressed())
    {
        ledOn();
        if (!buzzerController.isPlaying())
        {
            buzzerController.startMelody(*melody);
        }
    }
    else
    {
        ledOff();
    }
}