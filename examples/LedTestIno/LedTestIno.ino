/**
 * LedTestIno - State Machine LED Example for Arduino IDE
 *
 * Demonstrates the StateMachine framework with a button-controlled LED
 * that cycles through states: OFF -> ON -> SLOW_BLINK -> FAST_BLINK -> OFF
 *
 * Hardware:
 *   - LED on LED_PIN (default: built-in LED)
 *   - Button on BUTTON_PIN (default: GPIO 23)
 *
 * Required Libraries (install via Library Manager):
 *   - TaskScheduler by Anatoli Arkhipenko
 *   - OneButton by Matthias Hertel
 *   - StateMachine (this framework)
 */

// TaskScheduler configuration - must be before includes
#define _TASK_TIMEOUT
#define _TASK_SCHEDULING_OPTIONS
#define _TASK_THREAD_SAFE
#define _TASK_STATUS_REQUEST
#define _TASK_OO_CALLBACKS

#include <Arduino.h>
#include <OneButton.h>
#include "smMachine.h"

// ============================================================================
// Pin Definitions - Adjust for your board
// ============================================================================
#define LED_PIN     LED_BUILTIN   // Built-in LED (GPIO 2 on most ESP32)
#define BUTTON_PIN  23            // Button pin (active low with pullup)

// ============================================================================
// Exit Codes
// ============================================================================
#define EXIT_BUTTON_PRESS EXIT_USER

// ============================================================================
// LED Device Class
// ============================================================================
class LED : public smDevice {
public:
    LED(uint8_t pin) : mPin(pin), mLedOn(false) {}

    bool begin() override {
        pinMode(mPin, OUTPUT);
        off();
        return true;
    }

    bool start() override {
        off();
        mState = smON;
        return true;
    }

    void stop() override {
        off();
        mState = smOFF;
    }

    void end() override {
        off();
    }

    void on() {
        digitalWrite(mPin, HIGH);
        mLedOn = true;
    }

    void off() {
        digitalWrite(mPin, LOW);
        mLedOn = false;
    }

    void toggle() {
        if (mLedOn) {
            off();
        } else {
            on();
        }
    }

    bool isOn() { return mLedOn; }

private:
    uint8_t mPin;
    bool mLedOn;
};

// ============================================================================
// Button Device Class
// ============================================================================
class Button : public smDevice {
public:
    Button(uint8_t pin, bool activeLow = true, bool usePullup = true)
        : mButton(pin, activeLow, usePullup)
        , mClickedFlag(false)
        , mLongPressedFlag(false)
        , mDoubleClickedFlag(false)
    {
    }

    bool begin() override {
        mButton.attachClick(onClickStatic, this);
        mButton.attachLongPressStop(onLongPressStatic, this);
        mButton.attachDoubleClick(onDoubleClickStatic, this);
        mState = smOFF;
        return true;
    }

    bool start() override {
        mClickedFlag = false;
        mLongPressedFlag = false;
        mDoubleClickedFlag = false;
        mState = smON;
        return true;
    }

    void stop() override {
        mState = smOFF;
    }

    void end() override {
        stop();
    }

    void tick() {
        if (mState == smON) {
            mButton.tick();
        }
    }

    bool wasPressed() {
        if (mClickedFlag) {
            mClickedFlag = false;
            return true;
        }
        return false;
    }

    bool wasLongPressed() {
        if (mLongPressedFlag) {
            mLongPressedFlag = false;
            return true;
        }
        return false;
    }

    bool wasDoubleClicked() {
        if (mDoubleClickedFlag) {
            mDoubleClickedFlag = false;
            return true;
        }
        return false;
    }

private:
    static void onClickStatic(void* ptr) {
        static_cast<Button*>(ptr)->mClickedFlag = true;
    }

    static void onLongPressStatic(void* ptr) {
        static_cast<Button*>(ptr)->mLongPressedFlag = true;
    }

    static void onDoubleClickStatic(void* ptr) {
        static_cast<Button*>(ptr)->mDoubleClickedFlag = true;
    }

    OneButton mButton;
    volatile bool mClickedFlag;
    volatile bool mLongPressedFlag;
    volatile bool mDoubleClickedFlag;
};

// ============================================================================
// Base Action Class for LED + Button
// ============================================================================
class LedButtonAction : public smAction {
public:
    LedButtonAction(LED* led, Button* button)
        : smAction(led)
        , mLed(led)
        , mButton(button)
    {}

    bool begin() override {
        bool ok = true;
        if (mLed) ok &= mLed->begin();
        if (mButton) ok &= mButton->begin();
        return ok;
    }

    void end() override {
        if (mLed) mLed->end();
        if (mButton) mButton->end();
    }

protected:
    LED* mLed;
    Button* mButton;

    bool checkButton() {
        if (mButton) {
            mButton->tick();
            if (mButton->wasPressed()) {
                requestExit(EXIT_BUTTON_PRESS);
                return true;
            }
        }
        return false;
    }
};

// ============================================================================
// LED Off Action
// ============================================================================
class LedOffAction : public LedButtonAction {
public:
    LedOffAction(LED* led, Button* button) : LedButtonAction(led, button) {}

    void onEnter() override {
        if (mLed) mLed->off();
        if (mButton) mButton->start();
    }

    bool onRun() override {
        checkButton();
        return true;
    }

    void onExit() override {
        if (mButton) mButton->stop();
    }
};

// ============================================================================
// LED On Action
// ============================================================================
class LedOnAction : public LedButtonAction {
public:
    LedOnAction(LED* led, Button* button) : LedButtonAction(led, button) {}

    void onEnter() override {
        if (mLed) mLed->on();
        if (mButton) mButton->start();
    }

    bool onRun() override {
        checkButton();
        return true;
    }

    void onExit() override {
        if (mButton) mButton->stop();
    }
};

// ============================================================================
// LED Blink Action
// ============================================================================
class LedBlinkAction : public LedButtonAction {
public:
    LedBlinkAction(LED* led, Button* button, uint32_t intervalMs)
        : LedButtonAction(led, button)
        , mIntervalMs(intervalMs)
        , mLastToggleTime(0)
    {}

    void onEnter() override {
        if (mLed) mLed->on();
        if (mButton) mButton->start();
        mLastToggleTime = millis();
    }

    bool onRun() override {
        if ((millis() - mLastToggleTime) >= mIntervalMs) {
            if (mLed) mLed->toggle();
            mLastToggleTime = millis();
        }
        checkButton();
        return true;
    }

    void onExit() override {
        if (mButton) mButton->stop();
        if (mLed) mLed->off();
    }

private:
    uint32_t mIntervalMs;
    uint32_t mLastToggleTime;
};

// ============================================================================
// Global Instances
// ============================================================================

// Devices
LED led(LED_PIN);
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
    { &STATE_ON,         EXIT_TIMEOUT,      &STATE_OFF },
    { &STATE_SLOW_BLINK, EXIT_BUTTON_PRESS, &STATE_FAST_BLINK },
    { &STATE_FAST_BLINK, EXIT_BUTTON_PRESS, &STATE_OFF },
};
constexpr uint8_t NUM_TRANSITIONS = sizeof(transitions) / sizeof(transitions[0]);

// State machine
smMachine fsm(states, NUM_STATES, transitions, NUM_TRANSITIONS);

// ============================================================================
// Setup
// ============================================================================
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

// loop() is provided by smMachine.cpp - do not define here
