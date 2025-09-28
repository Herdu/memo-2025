#include "infrastructure.h"

// ----- Melodies (Distinct tones for each button) -----
int melody1Notes[] = { 262 }; // C4
int melody1Durations[] = { 200 };
Melody melody1(melody1Notes, melody1Durations, 1);

int melody2Notes[] = { 330 }; // E4  
int melody2Durations[] = { 200 };
Melody melody2(melody2Notes, melody2Durations, 1);

int melody3Notes[] = { 392 }; // G4
int melody3Durations[] = { 200 };
Melody melody3(melody3Notes, melody3Durations, 1);

int melody4Notes[] = { 523 }; // C5
int melody4Durations[] = { 200 };
Melody melody4(melody4Notes, melody4Durations, 1);

// ----- Global Objects -----
BuzzerController buzzerController(2);
LCD myLCD(0x27, 16, 2);

ButtonLed sets[] = {
  ButtonLed(6, 7, &melody1, 0),   // Button 0 (Red)
  ButtonLed(8, 9, &melody2, 1),   // Button 1 (Green)
  ButtonLed(10, 11, &melody3, 2), // Button 2 (Blue)
  ButtonLed(12, 13, &melody4, 3), // Button 3 (Yellow)
};
int buttonCount = 4;

// ----- Memory Game -----
MemoryGame game;

void setup() {
  Serial.begin(9600);
  Serial.println("Memory Game 2025 Starting...");
  
  // Setup hardware
  for (int i = 0; i < buttonCount; i++) {
    sets[i].setup();
  }
  myLCD.setup();
  buzzerController.setup();
  
  // Setup game
  game.setup();
  
  Serial.println("Game ready! Press any button to start.");
}

void loop() {
  // Update buzzer (for melody playback)
  buzzerController.update();
  
  // Update game logic
  game.update();
  
  // Debug: Print game state every 2 seconds
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 2000) {
    Serial.print("Game state: ");
    Serial.print(game.getState());
    Serial.print(", Level: ");
    Serial.println(game.getLevel());
    lastDebug = millis();
  }
  
  // Handle all buttons
  for (int i = 0; i < buttonCount; i++) {
    // During sequence display, don't let handle() control LEDs
    bool allowLedControl = !game.isShowingSequence();
    sets[i].handle(allowLedControl);
    
    // Check for button releases
    if (sets[i].wasReleased()) {
      Serial.print("Button ");
      Serial.print(i);
      Serial.println(" released");
      
      // Handle game input based on current state
      if (game.getState() == WAITING_TO_START) {
        // Start the game on any button press
        Serial.println("Starting game...");
        game.startSequence(); // Properly start the sequence
      } else if (game.isWaitingForInput()) {
        // Process player input during game
        Serial.print("Processing input: ");
        Serial.println(i);
        game.processPlayerInput(i);
      }
    }
  }
  
  // Small delay to prevent overwhelming the system
  delay(10);
}