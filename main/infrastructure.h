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
    void stopMelody();
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
    void clear();
};

// ----------------- Button + LED -----------------
class ButtonLed
{
private:
    int pinButton;
    int pinLed;
    const Melody *melody;
    bool lastButtonState = false;
    bool soundPlayed = false; // Flag to prevent sound spam
    int buttonId;

public:
    ButtonLed(int button, int led, const Melody *m, int id);

    void setup();
    bool isPressed();
    bool wasReleased();
    void ledOn();
    void ledOff();
    void handle();
    void handle(bool allowLedControl); // New overload
    void flashLed(int duration = 300);
    int getId() const;
};

// ----------------- Game States -----------------
enum GameState {
    WAITING_TO_START,
    SHOWING_SEQUENCE,
    WAITING_FOR_INPUT,
    GAME_OVER,
    LEVEL_COMPLETE
};

// ----------------- Memory Game -----------------
class MemoryGame
{
private:
    static const int MAX_SEQUENCE_LENGTH = 20;
    static const int MAX_BUTTONS = 4;
    
    int sequence[MAX_SEQUENCE_LENGTH];
    int sequenceLength;
    int currentStep;
    int playerInput;
    GameState gameState;
    unsigned long stateTimer;
    unsigned long levelStartTime;
    int score;
    int level;
    bool waitingForSequenceComplete;
    
    // Timing constants
    static const unsigned long SEQUENCE_DELAY = 800;
    static const unsigned long INPUT_TIMEOUT = 5000;
    static const unsigned long GAME_OVER_DISPLAY = 3000;
    static const unsigned long LEVEL_COMPLETE_DISPLAY = 2000;

public:
    MemoryGame();
    
    void setup();
    void update();
    void startNewGame();
    void startSequence();
    void generateNextSequence();
    void showSequence();
    void processPlayerInput(int buttonId);
    bool isWaitingForInput();
    void displayGameState();
    GameState getState();
    int getScore();
    int getLevel();
    bool isShowingSequence();
    void gameOver();
    void levelComplete();
};