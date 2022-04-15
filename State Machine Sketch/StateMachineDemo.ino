#include <Arduino.h>

// Define magic numbers into meaningful names
#define pushBtn 3
#define beepPin 13

#define redLED 10
#define grnLED 8
#define ylwLED 7

// Forward declaration of all functions
void blinkRedLed();
void blinkGrnLed();
void toggleYlwLED();
void displayState(String currState);

// One time set up for GPIO pins and UART (Serial)
void setup() {
    Serial.begin(115200);

    pinMode(redLED, OUTPUT);
    pinMode(grnLED, OUTPUT);
    pinMode(ylwLED, OUTPUT);

    // Push button is a pullup to avoid it floating around between 0 - 5v
    pinMode(pushBtn, INPUT_PULLUP);
    pinMode(beepPin, OUTPUT);

    digitalWrite(grnLED, LOW);
    digitalWrite(redLED, LOW);
    digitalWrite(ylwLED, LOW);

    // Seed the random number generator
    randomSeed(analogRead(A1));

    // All done
    Serial.println("Setup completed");
}

void loop() {
    // EVery 500mS
    blinkRedLed();

    // Every 100mS
    blinkGrnLed();

    // As often as possible?
    toggleYlwLED();
}

// Is it time to toggle the red LED?
void blinkRedLed() {
    static unsigned long redMillis = millis();

    if (millis() - redMillis > 500) {
        digitalWrite(redLED, !digitalRead(redLED));
        redMillis = millis();
    }
}

// Is it time to toggle the green LED?
void blinkGrnLed() {
    static unsigned long grnMillis = millis();

    if (millis() - grnMillis > 100) {
        digitalWrite(grnLED, !digitalRead(grnLED));
        grnMillis = millis();
    }
}

// A very small State Machine
void toggleYlwLED() {
    static unsigned long ylwMillis = millis();

    // Delay until elevator arrives
    static uint16_t elevatorDelay;

    // Timer for notification process has completed
    static unsigned long beepMillis;

    // Declare the states in meaningful English. Enums start enumerating
    // at zero, incrementing in steps of 1 unless overridden. We use an
    // enum 'class' here for type safety and code readability
    enum class elevatorState : uint8_t {
        IDLE,      // defaults to 0
        CALLED,    // defaults to 1
        ARRIVED,   // defaults to 2
        DOORSOPEN, // defaults to 3
    };

    // Keep track of the current State (it's an elevatorState variable)
    static elevatorState currState = elevatorState::IDLE;

    // Process according to our State Diagram
    switch (currState) {

        // Initial state (or final returned state)
        case elevatorState::IDLE:
            displayState("IDLE state");

            // Someone pushed the button yet?
            if (digitalRead(pushBtn) == LOW) {
                // Set the millis counter for the elevator arrival timer
                ylwMillis = millis();

                // Not really part of this process, simulates background action
                {
                    // Calculate random elevator arrival delay (seconds * milliSeconds)
                    elevatorDelay = random(3, 8) * 1000;
                    Serial.print("Elevator delay:");
                    Serial.println(elevatorDelay);
                }

                // Move to next state
                currState = elevatorState::CALLED;
            }
            break;

        // Someone pushed the 'call elevator' button - an input
        case elevatorState::CALLED:
            displayState("CALLED state");

            // Light the 'elevator called' LED
            digitalWrite(ylwLED, HIGH);

            // Has the elevator arrived yet?
            if (millis() - ylwMillis >= elevatorDelay) {
                // Move to next state
                currState = elevatorState::ARRIVED;
            }
            break;

        // Elevator has arrived
        case elevatorState::ARRIVED:
            displayState("ARRIVED State");

            // Quick beep to alert user that elevator has arrived
            digitalWrite(beepPin, HIGH);

            // Set the timer for the notification
            beepMillis = millis();

            // Move to next state
            currState = elevatorState::DOORSOPEN;
            break;

        case elevatorState::DOORSOPEN:
            displayState("DOORS OPEN state");

            // Extinguish the LED
            digitalWrite(ylwLED, LOW);

            // Time to turn off the beeper yet?
            if (millis() - beepMillis >= 100) {
                // Turn off beeper
                digitalWrite(beepPin, LOW);

                // Move to next state
                currState = elevatorState::IDLE;
            }
            break;

        default:
            // Nothing to do here
            Serial.println("'Default' Switch Case reached - Error");
    }
}

// Helper routine to track state machine progress
void displayState(String currState) {
    static String prevState = "";

    if (currState != prevState) {
        Serial.println(currState);
        prevState = currState;
    }
}