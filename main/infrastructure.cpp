#include "infrastructure.h"

extern LCD myLCD;
extern BuzzerController buzzerController;
extern ButtonLed sets[];

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

void BuzzerController::stopMelody()
{
    playing = false;
    currentMelody = nullptr;
    noTone(pin);
}

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

void LCD::clear()
{
    lcd.clear();
}

// ----------------- Button + LED -----------------
ButtonLed::ButtonLed(int button, int led, const Melody *m, int id)
{
    pinButton = button;
    pinLed = led;
    melody = m;
    buttonId = id;
    soundPlayed = false;
}

void ButtonLed::setup()
{
    pinMode(pinButton, INPUT_PULLUP);
    pinMode(pinLed, OUTPUT);
    digitalWrite(pinLed, HIGH); // LED off initially
    lastButtonState = digitalRead(pinButton);
}

bool ButtonLed::isPressed()
{
    return digitalRead(pinButton) == LOW;
}

bool ButtonLed::wasReleased()
{
    bool currentState = isPressed();
    bool released = false;
    
    if (lastButtonState == true && currentState == false) {
        released = true;
    }
    
    lastButtonState = currentState;
    return released;
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
    handle(true); // Default behavior - allow LED control
}

void ButtonLed::handle(bool allowLedControl)
{
    extern BuzzerController buzzerController; // declared in main file
    
    if (allowLedControl) {
        if (isPressed())
        {
            ledOn();
            // Play sound only once per button press (not continuously while held)
            if (!soundPlayed && !buzzerController.isPlaying()) {
                buzzerController.startMelody(*melody);
                soundPlayed = true;
            }
        }
        else
        {
            ledOff();
            soundPlayed = false; // Reset sound flag when button is released
        }
    } else {
        // During sequence display, still update button state but don't control LED/sound
        // Just call isPressed() to update the state tracking for wasReleased() to work
        isPressed();
    }
}

void ButtonLed::flashLed(int duration)
{
    ledOn();
    buzzerController.startMelody(*melody);
    // Note: LED will be turned off by the game logic after duration
}

int ButtonLed::getId() const
{
    return buttonId;
}

// ----------------- Memory Game -----------------
MemoryGame::MemoryGame()
{
    sequenceLength = 0;
    currentStep = 0;
    playerInput = 0;
    gameState = WAITING_TO_START;
    stateTimer = 0;
    score = 0;
    level = 1;
    waitingForSequenceComplete = false;
}

void MemoryGame::setup()
{
    randomSeed(analogRead(A0)); // Use analog noise for randomization
    startNewGame();
}

void MemoryGame::startNewGame()
{
    sequenceLength = 1;
    currentStep = 0;
    playerInput = 0;
    score = 0;
    level = 1;
    gameState = WAITING_TO_START;
    stateTimer = millis();
    waitingForSequenceComplete = false;
    
    // Generate first sequence
    generateNextSequence();
    
    myLCD.print("Memory Game 2025", "Press any button!");
}

void MemoryGame::startSequence()
{
    gameState = SHOWING_SEQUENCE;
    stateTimer = millis();
    myLCD.print("Level " + String(level), "Get ready...");
    delay(1500); // Give player time to prepare
    myLCD.print("Level " + String(level), "Watch sequence...");
}

void MemoryGame::generateNextSequence()
{
    if (sequenceLength < MAX_SEQUENCE_LENGTH) {
        int newButton;
        
        if (sequenceLength == 1) {
            // First button can be anything
            newButton = random(0, MAX_BUTTONS);
        } else {
            // Make sure new button is different from the previous one
            int lastButton = sequence[sequenceLength - 2];
            do {
                newButton = random(0, MAX_BUTTONS);
            } while (newButton == lastButton);
        }
        
        sequence[sequenceLength - 1] = newButton;
        
        Serial.print("Generated button ");
        Serial.print(newButton);
        Serial.print(" for position ");
        Serial.print(sequenceLength - 1);
        Serial.print(" (avoiding ");
        if (sequenceLength > 1) {
            Serial.print(sequence[sequenceLength - 2]);
        } else {
            Serial.print("none");
        }
        Serial.println(")");
    }
}

void MemoryGame::showSequence()
{
    static int sequenceStep = 0;
    static unsigned long lastFlashTime = 0;
    static bool ledState = false;
    static bool sequenceJustStarted = true;
    
    if (gameState != SHOWING_SEQUENCE) {
        sequenceStep = 0;
        ledState = false;
        sequenceJustStarted = true;
        return;
    }
    
    // Initialize timing when sequence starts
    if (sequenceJustStarted) {
        lastFlashTime = millis();
        sequenceJustStarted = false;
        sequenceStep = 0; // Make sure we start from beginning
        Serial.print("Starting sequence of length: ");
        Serial.println(sequenceLength);
    }
    
    unsigned long currentTime = millis();
    
    if (!ledState) {
        // Check if it's time for next step (or first step)
        if (sequenceStep == 0 || (currentTime - lastFlashTime >= 600)) { // Longer delay between steps
            if (sequenceStep < sequenceLength) {
                Serial.print("Showing step ");
                Serial.print(sequenceStep);
                Serial.print(" - Button ");
                Serial.println(sequence[sequenceStep]);
                
                sets[sequence[sequenceStep]].flashLed();
                ledState = true;
                lastFlashTime = currentTime;
            } else {
                // Sequence complete
                Serial.println("Sequence complete - waiting for input");
                gameState = WAITING_FOR_INPUT;
                currentStep = 0;
                playerInput = 0;
                stateTimer = currentTime;
                sequenceStep = 0;
                sequenceJustStarted = true;
                myLCD.print("Your turn!", "Level " + String(level));
            }
        }
    } else {
        // Turn off LED after flash duration
        if (currentTime - lastFlashTime >= 400) { // Longer flash duration
            sets[sequence[sequenceStep]].ledOff();
            ledState = false;
            lastFlashTime = currentTime;
            sequenceStep++;
            
            Serial.print("Step ");
            Serial.print(sequenceStep - 1);
            Serial.println(" complete");
        }
    }
}

void MemoryGame::processPlayerInput(int buttonId)
{
    if (gameState != WAITING_FOR_INPUT) return;
    
    if (sequence[currentStep] == buttonId) {
        // Correct input
        currentStep++;
        score += 10;
        
        if (currentStep >= sequenceLength) {
            // Level complete!
            levelComplete();
        } else {
            // Continue with next step
            stateTimer = millis(); // Reset input timeout
        }
    } else {
        // Wrong input - game over
        gameOver();
    }
}

bool MemoryGame::isWaitingForInput()
{
    return gameState == WAITING_FOR_INPUT;
}

bool MemoryGame::isShowingSequence()
{
    return gameState == SHOWING_SEQUENCE;
}

void MemoryGame::update()
{
    unsigned long currentTime = millis();
    
    switch (gameState) {
        case WAITING_TO_START:
            // Wait for any button press to start
            break;
            
        case SHOWING_SEQUENCE:
            showSequence();
            break;
            
        case WAITING_FOR_INPUT:
            // Check for input timeout
            if (currentTime - stateTimer > INPUT_TIMEOUT) {
                gameOver();
            }
            break;
            
        case LEVEL_COMPLETE:
            if (currentTime - stateTimer > LEVEL_COMPLETE_DISPLAY) {
                level++;
                sequenceLength++;
                generateNextSequence();
                startSequence(); // Use the proper method
            }
            break;
            
        case GAME_OVER:
            if (currentTime - stateTimer > GAME_OVER_DISPLAY) {
                startNewGame();
            }
            break;
    }
}

void MemoryGame::levelComplete()
{
    gameState = LEVEL_COMPLETE;
    stateTimer = millis();
    score += level * 20; // Bonus points
    myLCD.print("Level Complete!", "Score: " + String(score));
    
    // Play success sound
    buzzerController.playTone(523, 150); // C5
    delay(150);
    buzzerController.playTone(659, 150); // E5
    delay(150);
    buzzerController.playTone(784, 300); // G5
}

void MemoryGame::gameOver()
{
    gameState = GAME_OVER;
    stateTimer = millis();
    myLCD.print("Game Over!", "Score: " + String(score));
    
    // Play game over sound
    buzzerController.playTone(196, 400); // G3
    delay(400);
    buzzerController.playTone(147, 600); // D3
    
    // Flash all LEDs
    for (int i = 0; i < MAX_BUTTONS; i++) {
        sets[i].ledOn();
    }
    delay(500);
    for (int i = 0; i < MAX_BUTTONS; i++) {
        sets[i].ledOff();
    }
}

GameState MemoryGame::getState()
{
    return gameState;
}

int MemoryGame::getScore()
{
    return score;
}

int MemoryGame::getLevel()
{
    return level;
}