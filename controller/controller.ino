#include "controller-info.hpp"

// Button info.
const int pins[NUM_BUTTONS] = {2,4,6,8,12};
byte      states;

void setup()
{
    // Button setup.
    for (int i = 0; i < NUM_BUTTONS; i++)
    {
        pinMode(pins[i], INPUT_PULLUP);
    }

    // Used for status.
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Starts Serial and waits until it's ready.
    Serial.begin(BAUD_RATE);
    while (!Serial);
}

// Time of the last input.
long lastInput;
// Time, in ms, that the LED stays on after an input.
long keepAlive = 1000;

void loop()
{
    // Read button states.
    states = 0;
    for (int i = 0; i < NUM_BUTTONS; i++)
    {
        states += (digitalRead(pins[i]) ? 0 : 1) << i;
    }

    // Writes state.
    Serial.write(states);

    // Waits for confirmation.
    lastInput = millis();
    while (Serial.read() != CONFIRMATION_BYTE)
    {
        // Disables LED after some time without inputs.
        if (millis() - lastInput > keepAlive)
        {
            digitalWrite(LED_BUILTIN, LOW);
        }
    }
    digitalWrite(LED_BUILTIN, HIGH);
}
