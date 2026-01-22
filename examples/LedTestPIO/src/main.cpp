#include <Arduino.h>
#include "StateMachine.h"
#include "LED.h"
#include "Button.h"
#include "LedActions.h"

// Pin definitions
#define BUTTON_PIN  23

// Devices
LED led;  // TinyPICO DotStar LED, default green
Button button(BUTTON_PIN, true, true);  // Active low, use pullup

// Actions
LedOffAction   actionOff(&led, &button);
LedOnAction    actionOn(&led, &button);
LedBlinkAction actionSlowBlink(&led, &button, 500);  // 500ms interval
LedBlinkAction actionFastBlink(&led, &button, 100);  // 100ms interval

// States
smState STATE_OFF(&actionOff);
smState STATE_ON(&actionOn);
smState STATE_SLOW_BLINK(&actionSlowBlink);
smState STATE_FAST_BLINK(&actionFastBlink);

smState* states[] = {
    &STATE_OFF,
    &STATE_ON,
    &STATE_SLOW_BLINK,
    &STATE_FAST_BLINK
};
constexpr uint8_t NUM_STATES = sizeof(states) / sizeof(states[0]);

// Transitions: button press cycles through states
//   OFF -> ON -> SLOW_BLINK -> FAST_BLINK -> OFF
smTransition transitions[] = {
    { &STATE_OFF,        EXIT_BUTTON_PRESS, &STATE_ON },
    { &STATE_ON,         EXIT_BUTTON_PRESS, &STATE_SLOW_BLINK },
    { &STATE_ON,         EXIT_TIMEOUT, &STATE_OFF },
    { &STATE_SLOW_BLINK, EXIT_BUTTON_PRESS, &STATE_FAST_BLINK },
    { &STATE_FAST_BLINK, EXIT_BUTTON_PRESS, &STATE_OFF },
};
constexpr uint8_t NUM_TRANSITIONS = sizeof(transitions) / sizeof(transitions[0]);

// State machine
smMachine fsm(states, NUM_STATES, transitions, NUM_TRANSITIONS);

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("LED State Machine Example");
    Serial.println("Press button to cycle: OFF -> ON -> SLOW -> FAST -> OFF");

    if (!fsm.begin()) {
        Serial.println("ERROR: FSM begin failed!");
        return;
    }

    if (!fsm.start(&STATE_OFF)) {
        Serial.println("ERROR: FSM start failed!");
        return;
    }

    STATE_ON.setTimeout(5000);  // Auto-exit ON state after 5 seconds

    Serial.println("FSM started in OFF state");
}

// loop() is provided by smMachine.cpp
