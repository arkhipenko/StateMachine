# LED State Machine Example - Development Guide

This guide explains how the LED state machine example works and how to extend it for your own projects. It covers the framework architecture, code organization, and common patterns for building state-driven embedded applications.

## Table of Contents

1. [What This Example Does](#1-what-this-example-does)
2. [Project Structure](#2-project-structure)
3. [The Big Picture - How State Machines Work](#3-the-big-picture---how-state-machines-work)
4. [Setting Up Your Environment](#4-setting-up-your-environment)
5. [Understanding the Code Layer by Layer](#5-understanding-the-code-layer-by-layer)
6. [Adding Your Own States](#6-adding-your-own-states)
7. [Adding New Hardware (Devices)](#7-adding-new-hardware-devices)
8. [Common Patterns and Tricks](#8-common-patterns-and-tricks)
9. [Debugging Tips](#9-debugging-tips)
10. [Troubleshooting](#10-troubleshooting)

---

## 1. What This Example Does

This example implements a simple LED controller that cycles through four states when a button is pressed:

```
OFF -> ON -> SLOW_BLINK -> FAST_BLINK -> OFF -> ...
```

The ON state also has a 5-second timeout. If the button isn't pressed within that time, the state machine automatically transitions back to OFF.

**Hardware used:**
- TinyPICO board (ESP32-based)
- Built-in DotStar RGB LED
- External button on GPIO 23

The framework itself works with any ESP32 board - you'll just need to swap out the TinyPICO LED library for your specific hardware.

---

## 2. Project Structure

```
LedTestPIO/
    platformio.ini      <- Build config, dependencies, flags
    include/
        LED.h           <- LED device driver header
        Button.h        <- Button device driver header
        LedActions.h    <- Action classes header
    src/
        main.cpp        <- Setup, state/transition definitions
        LED.cpp         <- LED driver implementation
        Button.cpp      <- Button driver implementation
        LedActions.cpp  <- Action implementations
    lib/                <- (empty, deps pulled from registry)
    test/               <- (empty for now)
```

The state machine framework itself lives in a seperate library (StateMachineFramework) which is pulled in via platformio.ini.

---

## 3. The Big Picture - How State Machines Work

The framework is built around four core concepts, arranged in a hierarchy:

```
smDevice   - Hardware driver (LED, button, motor, sensor...)
    |
    v
smAction   - An operation performed on devices (blink, fade, read...)
    |
    v
smState    - A Task that runs one action until an exit condition occurs
    |
    v
smMachine  - Central orchestrator that manages states and transitions
```

To understand the relationship:
- **Devices** represent your physical hardware
- **Actions** define what operations to perform on that hardware
- **States** determine when those actions run
- **Machine** decides what state comes next based on exit conditions

### The Transition Table

The transition table is the central concept of this framework. It defines rules in the form:

> "If the machine is in state X and exit code Y occurs, transition to state Z"

For example:

```cpp
{ &STATE_OFF, EXIT_BUTTON_PRESS, &STATE_ON }
```

This rule means: "when in the OFF state and a button press exit code is signaled, transition to the ON state."

The table is simply an array of these rules. When an action signals an exit, the machine looks up the current state and exit code to determine the next state.

### Exit Codes

Actions signal why they want to exit using predefined codes:

| Code | Name | Description |
|------|------|-------------|
| 0 | `EXIT_NONE` | No exit requested (keep running) |
| 1 | `EXIT_COMPLETE` | Normal completion |
| 2 | `EXIT_TIMEOUT` | State timed out (auto-detected by framework) |
| 3 | `EXIT_ERROR` | An error occurred |
| 4 | `EXIT_CANCEL` | Cancelled by request |
| 5 | `EXIT_ABORT` | Emergency stop |
| 16+ | `EXIT_USER` | Application-specific codes start here |

In this example, we define a custom exit code:

```cpp
#define EXIT_BUTTON_PRESS  EXIT_USER
```

This maps button press events to exit code 16.

One notable feature: `EXIT_TIMEOUT` is detected automatically by the framework when you set a timeout on a state. There's no need to implement timer logic in your actions.

---

## 4. Setting Up Your Environment

**Prerequisites:**
- PlatformIO (VS Code extension or CLI)
- USB cable
- TinyPICO or compatible ESP32 board

If you're using a diffrent board, update platformio.ini:

```ini
board = pico32          ; change to your board
```

You'll also need to replace the TinyPICO LED library with an appropriate driver for your hardware.

### Building

From the command line:

```bash
pio run -d examples/LedTestPIO
```

Or use the build button in VS Code.

### Uploading

```bash
pio run -d examples/LedTestPIO -t upload
```

Ensure your board is connected. If upload fails, try holding the boot button while the upload process attempts to connect.

### Monitoring Serial Output

```bash
pio device monitor -d examples/LedTestPIO
```

Expected output on startup:

```
LED State Machine Example
Press button to cycle: OFF -> ON -> SLOW -> FAST -> OFF
FSM started in OFF state
>> Entering STATE_OFF
```

When you press the button:

```
[Button pressed - requesting exit]
<< Exiting STATE_OFF (BUTTON_PRESS)
>> Entering STATE_ON (timeout=5s)
```

### Build Flags

These flags in platformio.ini are **required** for the framework to function correctly:

```ini
build_flags =
    -D _TASK_TIMEOUT             ; Enables task timeouts
    -D _TASK_SCHEDULING_OPTIONS  ; Multiple scheduling modes
    -D _TASK_THREAD_SAFE         ; Thread safety (important for ESP32)
    -D _TASK_STATUS_REQUEST      ; Event-triggered tasks
    -D _TASK_OO_CALLBACKS        ; Inheritance-based callbacks
    -D _TASK_HEADER_AND_CPP      ; Split header/implementation
```

Missing these flags will result in compile errors or unexpected runtime behavior.

---

## 5. Understanding the Code Layer by Layer

This section walks through the code from the lowest level (hardware) to the highest (state machine).

### Layer 1: Devices (LED.h/cpp, Button.h/cpp)

Devices inherit from `smDevice` and implement four lifecycle methods:

| Method | Purpose |
|--------|---------|
| `bool begin()` | Initialize hardware (called once at startup) |
| `bool start()` | Activate the device |
| `void stop()` | Deactivate the device |
| `void end()` | Release hardware resources (cleanup) |

The LED class implementation:

```cpp
class LED : public smDevice {
public:
    LED(uint8_t r = 0, uint8_t g = 255, uint8_t b = 0);  // Default green

    bool begin() override;
    bool start() override;
    void stop() override;
    void end() override;

    // Device-specific operations
    void on();
    void off();
    void toggle();
    void setColor(uint8_t r, uint8_t g, uint8_t b);
};
```

Each device also tracks its operational state (`smON`, `smOFF`, `smSTARTING`, `smSTOPPING`), which is useful for debugging.

The Button class wraps the OneButton library:

```cpp
class Button : public smDevice {
    void tick();           // Must be called regularly
    bool wasPressed();     // Check & clear pressed flag
    bool wasLongPressed();
    bool wasDoubleClicked();
};
```

**Important:** The `tick()` method must be called on every loop iteration for button detection to work. The action's `onRun()` method is the appropriate place for this call.

### Layer 2: Actions (LedActions.h/cpp)

Actions inherit from `smAction` and implement these methods:

| Method | Purpose |
|--------|---------|
| `bool begin()` | Initialize (called once) |
| `void end()` | Cleanup (called once) |
| `void onEnter()` | Called when state becomes active |
| `bool onRun()` | Called repeatedly while active (return true to continue) |
| `void onExit()` | Called when state becomes inactive |

The base class `LedButtonAction` handles common functionality:

```cpp
class LedButtonAction : public smAction {
protected:
    LED* mLed;
    Button* mButton;

    bool checkButton() {
        mButton->tick();
        if (mButton->wasPressed()) {
            requestExit(EXIT_BUTTON_PRESS);  // Triggers state transition
            return true;
        }
        return false;
    }
};
```

Specific actions override the base behavior:

```cpp
class LedBlinkAction : public LedButtonAction {
    void onEnter() override {
        mLed->on();
        mButton->start();
        mLastToggleTime = millis();
    }

    bool onRun() override {
        // Toggle LED at interval
        if ((millis() - mLastToggleTime) >= mIntervalMs) {
            mLed->toggle();
            mLastToggleTime = millis();
        }
        checkButton();  // Check for exit condition
        return true;    // Continue running
    }

    void onExit() override {
        mLed->off();
        mButton->stop();
    }
};
```

### Key Concept: requestExit()

When an action needs to exit, it calls:

```cpp
requestExit(EXIT_BUTTON_PRESS);  // or any exit code
```

This signals to the state machine that the current state should end, along with the reason why. The machine then consults the transition table to determine the next state.

### Layer 3: States (main.cpp)

States are wrappers around actions. Each state contains exactly one action.

```cpp
// Create actions
LedOffAction   actionOff(&led, &button);
LedOnAction    actionOn(&led, &button);
LedBlinkAction actionSlowBlink(&led, &button, 500);
LedBlinkAction actionFastBlink(&led, &button, 100);

// Create states (wrap the actions)
smState STATE_OFF(&actionOff);
smState STATE_ON(&actionOn);
smState STATE_SLOW_BLINK(&actionSlowBlink);
smState STATE_FAST_BLINK(&actionFastBlink);
```

By convention, state variables use UPPER_CASE naming since they act as constants that identify each state.

### Layer 4: Transitions (main.cpp)

The transition table defines all valid state changes:

```cpp
smTransition transitions[] = {
    { &STATE_OFF,        EXIT_BUTTON_PRESS, &STATE_ON },
    { &STATE_ON,         EXIT_BUTTON_PRESS, &STATE_SLOW_BLINK },
    { &STATE_ON,         EXIT_TIMEOUT,      &STATE_OFF },  // Auto timeout
    { &STATE_SLOW_BLINK, EXIT_BUTTON_PRESS, &STATE_FAST_BLINK },
    { &STATE_FAST_BLINK, EXIT_BUTTON_PRESS, &STATE_OFF },
};
```

Note that `STATE_ON` has two outgoing transitions:
- Button press leads to SLOW_BLINK
- Timeout leads to OFF

Multiple exit conditions from a single state is a common and supported pattern.

### Layer 5: The Machine (main.cpp)

Finally, the machine is created and started:

```cpp
smState* states[] = { &STATE_OFF, &STATE_ON, &STATE_SLOW_BLINK, &STATE_FAST_BLINK };
constexpr uint8_t NUM_STATES = sizeof(states) / sizeof(states[0]);
constexpr uint8_t NUM_TRANSITIONS = sizeof(transitions) / sizeof(transitions[0]);

smMachine fsm(states, NUM_STATES, transitions, NUM_TRANSITIONS);

void setup() {
    Serial.begin(115200);

    fsm.begin();                  // Initialize all states and actions
    fsm.start(&STATE_OFF);        // Start in OFF state

    STATE_ON.setTimeout(5000);    // 5-second timeout on ON state
}

// Note: No loop() function needed - the framework provides it
```

---

## 6. Adding Your Own States

This section demonstrates how to add a new PULSE state where the LED fades in and out.

### Step 1: Create the action class (in LedActions.h)

```cpp
class LedPulseAction : public LedButtonAction {
public:
    LedPulseAction(LED* aLed, Button* aButton, uint32_t aPeriodMs);

    void onEnter() override;
    bool onRun() override;
    void onExit() override;

private:
    uint32_t mPeriodMs;
    uint32_t mStartTime;
};
```

### Step 2: Implement the action (in LedActions.cpp)

```cpp
LedPulseAction::LedPulseAction(LED* aLed, Button* aButton, uint32_t aPeriodMs)
    : LedButtonAction(aLed, aButton)
    , mPeriodMs(aPeriodMs)
    , mStartTime(0)
{
}

void LedPulseAction::onEnter() {
    Serial.println(">> Entering STATE_PULSE");
    if (mButton) mButton->start();
    mStartTime = millis();
}

bool LedPulseAction::onRun() {
    // Calculate brightness using sine wave
    uint32_t elapsed = millis() - mStartTime;
    float phase = (float)(elapsed % mPeriodMs) / mPeriodMs;
    uint8_t brightness = (uint8_t)(127.5 * (1.0 + sin(phase * 2 * PI)));

    if (mLed) mLed->setBrightness(brightness);

    checkButton();
    return true;
}

void LedPulseAction::onExit() {
    Serial.println("<< Exiting STATE_PULSE");
    if (mButton) mButton->stop();
    if (mLed) {
        mLed->setBrightness(128);  // Reset to default
        mLed->off();
    }
}
```

### Step 3: Register the state (in main.cpp)

```cpp
// Add action instance
LedPulseAction actionPulse(&led, &button, 2000);  // 2-second period

// Add state
smState STATE_PULSE(&actionPulse);

// Update states array
smState* states[] = {
    &STATE_OFF, &STATE_ON, &STATE_SLOW_BLINK, &STATE_FAST_BLINK, &STATE_PULSE
};

// Update transitions (insert pulse between fast blink and off)
smTransition transitions[] = {
    { &STATE_OFF,        EXIT_BUTTON_PRESS, &STATE_ON },
    { &STATE_ON,         EXIT_BUTTON_PRESS, &STATE_SLOW_BLINK },
    { &STATE_ON,         EXIT_TIMEOUT,      &STATE_OFF },
    { &STATE_SLOW_BLINK, EXIT_BUTTON_PRESS, &STATE_FAST_BLINK },
    { &STATE_FAST_BLINK, EXIT_BUTTON_PRESS, &STATE_PULSE },      // Modified
    { &STATE_PULSE,      EXIT_BUTTON_PRESS, &STATE_OFF },        // New
};
```

Rebuild the project and the new state will be active.

---

## 7. Adding New Hardware (Devices)

This section demonstrates adding a buzzer device that can beep in certain states.

### Step 1: Create the device class

**include/Buzzer.h:**

```cpp
#pragma once
#include "smDevice.h"

class Buzzer : public smDevice {
public:
    Buzzer(uint8_t aPin);

    bool begin() override;
    bool start() override;
    void stop() override;
    void end() override;

    void beep(uint32_t durationMs);
    void tone(uint32_t frequency);
    void noTone();

private:
    uint8_t mPin;
};
```

**src/Buzzer.cpp:**

```cpp
#include "Buzzer.h"

Buzzer::Buzzer(uint8_t aPin) : mPin(aPin) {}

bool Buzzer::begin() {
    pinMode(mPin, OUTPUT);
    digitalWrite(mPin, LOW);
    return true;
}

bool Buzzer::start() {
    mState = smON;
    return true;
}

void Buzzer::stop() {
    noTone();
    mState = smOFF;
}

void Buzzer::end() {
    stop();
}

void Buzzer::beep(uint32_t durationMs) {
    // Note: This blocking implementation is for demonstration only.
    // Production code should use non-blocking timer-based approach.
    tone(1000);
    delay(durationMs);
    noTone();
}

void Buzzer::tone(uint32_t frequency) {
    ledcWriteTone(0, frequency);  // ESP32 LEDC peripheral
}

void Buzzer::noTone() {
    ledcWriteTone(0, 0);
}
```

### Step 2: Integrate with actions

There are two approaches:

**Option A:** Add buzzer as an optional parameter to existing actions:

```cpp
class LedButtonAction : public smAction {
public:
    LedButtonAction(LED* aLed, Button* aButton, Buzzer* aBuzzer = nullptr);
    // ...
protected:
    Buzzer* mBuzzer;  // Optional buzzer
};
```

Then use it in specific actions:

```cpp
void LedOnAction::onEnter() {
    if (mBuzzer) mBuzzer->tone(440);  // A4 note
    // ...
}
```

**Option B:** Create dedicated actions that incorporate the buzzer.

---

## 8. Common Patterns and Tricks

### Pattern: Multiple exit conditions from one action

To handle both button press and long press with different behaviors:

```cpp
#define EXIT_BUTTON_PRESS      EXIT_USER
#define EXIT_BUTTON_LONG_PRESS (EXIT_USER + 1)

bool LedOnAction::onRun() {
    mButton->tick();

    if (mButton->wasPressed()) {
        requestExit(EXIT_BUTTON_PRESS);
    }
    else if (mButton->wasLongPressed()) {
        requestExit(EXIT_BUTTON_LONG_PRESS);
    }

    return true;
}
```

Then define transitions for both:

```cpp
{ &STATE_ON, EXIT_BUTTON_PRESS,      &STATE_SLOW_BLINK },
{ &STATE_ON, EXIT_BUTTON_LONG_PRESS, &STATE_OFF },
```

### Pattern: Self-transitions (restart current state)

To restart a state, create a transition that points back to itself:

```cpp
{ &STATE_BLINK, EXIT_RESTART, &STATE_BLINK }
```

The state will call `onExit()` followed by `onEnter()`, effectively restarting.

### Pattern: Adding background tasks

To run a task independently of state machine states, add it directly to the scheduler:

```cpp
Task heartbeatTask(1000, TASK_FOREVER, &heartbeatCallback);

void setup() {
    fsm.begin();
    fsm.getScheduler().addTask(heartbeatTask);
    heartbeatTask.enable();
    fsm.start(&STATE_OFF);
}

void heartbeatCallback() {
    Serial.println("Heartbeat...");
}
```

### Pattern: Accessing the machine from an action

Actions maintain a pointer to their parent machine:

```cpp
bool MyAction::onRun() {
    smMachine* machine = getMachine();

    unsigned long transitions = machine->getTransitionCount();
    smState* current = machine->getCurrentState();
    smState* previous = machine->getPreviousState();

    // ...
}
```

### Pattern: Handling invalid transitions

When an exit code has no matching transition, the framework calls `onInvalidTransition()`. Override this method to implement custom error handling:

```cpp
void MyAction::onInvalidTransition(uint8_t exitCode) {
    Serial.print("Unhandled exit code: ");
    Serial.println(exitCode);
    // Force transition to error state
    getMachine()->forceTransitionTo(&STATE_ERROR);
}
```

You can also override this method in the machine class for global handling.

### Pattern: Programmatic transitions

For transitions that depend on runtime conditions rather than just exit codes, use `forceTransitionTo()`:

```cpp
bool MyAction::onRun() {
    if (someComplexCondition()) {
        getMachine()->forceTransitionTo(&STATE_SPECIAL);
        return false;  // Stop running
    }
    return true;
}
```

Use this sparingly - it bypasses the transition table and can make the state machine harder to understand and maintain.

---

## 9. Debugging Tips

### Serial Logging

The example includes logging in `onEnter()` and `onExit()`. For additional visibility:

```cpp
bool LedBlinkAction::onRun() {
    static uint32_t lastLog = 0;
    if (millis() - lastLog > 1000) {
        Serial.print("Blink running, LED is ");
        Serial.println(mLed->isOn() ? "ON" : "OFF");
        lastLog = millis();
    }
    // ...
}
```

Avoid logging on every iteration - this will flood the serial port and impact performance.

### Monitoring Transition Count

```cpp
void logStats() {
    Serial.print("Transitions so far: ");
    Serial.println(fsm.getTransitionCount());
}
```

### Checking Current and Previous State

```cpp
void logState() {
    Serial.print("Current: ");
    Serial.println(fsm.getCurrentState()->getName());
    Serial.print("Previous: ");
    Serial.println(fsm.getPreviousState()->getName());
}
```

For this to work well, provide names when constructing states:

```cpp
smState STATE_OFF(&actionOff, "OFF");
```

### Timing Issues

If states transition too quickly or slowly, check:

1. The state's execution interval (default is 1ms):
   ```cpp
   smState STATE_FOO(&action, "FOO", 10);  // 10ms interval
   ```

2. Timeout values:
   ```cpp
   STATE_ON.setTimeout(5000);
   ```

3. Action logic - ensure `requestExit()` isn't being called prematurely.

### Button Debouncing

The OneButton library handles debouncing automatically. If you experience double triggers, adjust the timing:

```cpp
mButton.setDebounceMs(50);      // Default is 50ms
mButton.setClickMs(400);        // Time for click detection
mButton.setPressMs(800);        // Time for long press
```

---

## 10. Troubleshooting

### Problem: Compile error about undefined _TASK_OO_CALLBACKS

**Solution:** Add the required build flags to platformio.ini:

```ini
build_flags =
    -D _TASK_TIMEOUT
    -D _TASK_OO_CALLBACKS
    ... (see section 4 for complete list)
```

### Problem: Nothing happens when the button is pressed

**Possible causes:**
1. Button pin doesn't match wiring
2. Incorrect active low / pullup settings in Button constructor
3. `tick()` not being called in `onRun()`
4. Button hardware issue - add debug prints in `wasPressed()` to verify detection

### Problem: States transition immediately without waiting

**Cause:** The action is calling `requestExit()` in `onEnter()` or unconditionally in `onRun()`. Review the action logic.

### Problem: Timeout doesn't work

**Checklist:**
1. Set timeout after `fsm.begin()` completes
2. Verify `-D _TASK_TIMEOUT` is in build flags
3. Confirm theres a transition defined for `EXIT_TIMEOUT` from that state

### Problem: LED doesn't light up

**Checklist:**
1. Verify TinyPICO library is installed
2. Test with the basic TinyPICO example sketch
3. Confirm `mLed` pointer isn't null in your action

### Problem: Multiple button presses register as one

**Cause:** The `wasPressed()` method clears its flag when called. If called from multiple locations, only the first call will detect the press. Review your code flow.

### Problem: Program crashes or reboots

**Possible causes:**
1. Null pointer dereferences
2. Stack overflow (excessive recursion or large local arrays)
3. Enable debug output for more information:
   ```ini
   build_flags =
       -D CORE_DEBUG_LEVEL=5
   ```

### Problem: State machine appears stuck

**Checklist:**
1. Add logging to `onRun()` to verify it's being called
2. Check that the transition table has a valid exit path
3. Confirm `requestExit()` is actually being called

---

## Conclusion

This guide has covered the architecture and implementation of the LED state machine example. The framework is flexible enough to handle applications ranging from simple LED controllers to complex automation systems.

If you encounter issues not covered here, the source code is the definitive reference. Review `smAction.h`, `smState.h`, and `smMachine.h` for implementation details.
