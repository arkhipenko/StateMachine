# StateMachine

Lightweight, table-driven state machine framework built on [TaskScheduler](https://github.com/arkhipenko/TaskScheduler).

## Overview

StateMachine provides a structured approach to building cooperative, non-blocking state machines for Arduino and compatible microcontrollers. It separates state behavior (actions) from state management (machine) using a table-driven transition model.

---
#### Delivering robust embedded systems and firmware that perform flawlessly in real-world conditions.
[smart solutions for smart devices](https://smart4smart.com/)

[![github](https://github.com/arkhipenko/resources/blob/master/smart4smart_large.gif)](https://smart4smart.com/)
---

## Features

- **Table-driven transitions** - Define state flows declaratively with `{fromState, exitCode, toState}` tuples
- **Cooperative multitasking** - Built on TaskScheduler for non-blocking execution
- **Lifecycle hooks** - `onEnter()`, `onRun()`, `onExit()` for clean state management
- **Device abstraction** - Optional `smDevice` interface for hardware encapsulation
- **Configurable timing** - Per-state execution intervals and iteration limits
- **Standard exit codes** - Built-in codes for common conditions, extensible for application-specific needs

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| [TaskScheduler](https://github.com/arkhipenko/TaskScheduler) | ^4.0.4 | Cooperative multitasking foundation |

## Installation

### PlatformIO

Add to `platformio.ini`:

```ini
lib_deps =
    arkhipenko/TaskScheduler@^4.0.4
```

Place this library in your project's `lib/` folder.

### Arduino IDE

1. Install TaskScheduler via Library Manager
2. Copy this folder to your `libraries/` directory

## Architecture

The framework provides four core abstractions:

```
┌─────────────────────────────────────────────────────────────┐
│                        smMachine                            │
│  - Owns scheduler and transition table                      │
│  - Orchestrates state transitions based on exit codes       │
├─────────────────────────────────────────────────────────────┤
│                         smState                             │
│  - Wraps an smAction                                        │
│  - Extends TaskScheduler Task                               │
│  - Manages action lifecycle (enable/disable)                │
├─────────────────────────────────────────────────────────────┤
│                        smAction                             │
│  - Implements state behavior                                │
│  - Lifecycle: onEnter() → onRun() → onExit()                │
│  - Signals transitions via requestExit(code)                │
├─────────────────────────────────────────────────────────────┤
│                        smDevice                             │
│  - Optional hardware abstraction                            │
│  - Lifecycle: begin() → start() → stop() → end()            │
└─────────────────────────────────────────────────────────────┘
```

### smDevice

Base interface for hardware abstraction. Actions can optionally own a device and delegate lifecycle calls.

```cpp
class smDevice {
public:
    virtual bool begin() = 0;   // Initialize hardware
    virtual bool start() = 0;   // Activate device
    virtual void stop() = 0;    // Deactivate device
    virtual void end() = 0;     // Release hardware

    smDeviceState_t getState(); // smON, smOFF, smSTARTING, smSTOPPING
    const char* getName();

protected:
    void setState(smDeviceState_t state);  // For derived classes to update state
};
```

### smAction

Base class for state behavior. Implement the three lifecycle hooks:

```cpp
class smAction {
public:
    // Called once when state becomes active
    virtual void onEnter() {}

    // Called repeatedly while state is active
    // Call requestExit(code) to trigger transition
    virtual bool onRun() = 0;

    // Called once when leaving state
    virtual void onExit() {}

    // Called if exit code has no matching transition
    virtual void onInvalidTransition(uint8_t exitCode) {}

    // Signal state machine to transition
    void requestExit(uint8_t exitCode);

    // Exit code management
    uint8_t getExitCode();
    void resetExitCode();  // Called automatically on state entry

    // Access parent machine
    smMachine* getMachine();
};
```

### smState

Wraps an action and integrates with TaskScheduler:

```cpp
smState STATE_IDLE(&actionIdle, "IDLE");
smState STATE_RUNNING(&actionRunning, "RUNNING", 10, TASK_FOREVER);
//                                     name     interval  iterations
```

Parameters:
- `action` - Pointer to smAction implementation
- `name` - State name for logging/debugging
- `interval` - Execution interval in milliseconds (default: `SM_DEFAULT_INTERVAL_MS` = 1ms)
- `iterations` - Number of iterations before auto-exit (default: TASK_FOREVER)

Additional methods:
- `getEnterTime()` - Returns `millis()` timestamp when state was entered (useful for timeouts)
- `getAction()` - Returns pointer to the wrapped action
- `getName()` - Returns state name

### smMachine

Orchestrates state transitions using a table-driven approach:

```cpp
smMachine machine(states, numStates, transitions, numTransitions);

// Initialize all states
machine.begin();

// Start execution from initial state
machine.start(&STATE_IDLE);

// NOTE: The library provides its own loop() implementation.
// Do not define loop() in your sketch - the machine executes automatically.
```

Additional methods:
- `stop()` - Stop the machine (disables current state)
- `isRunning()` - Returns true if machine is running
- `getCurrentState()` - Returns pointer to current state
- `getPreviousState()` - Returns pointer to previous state (for transition context)
- `getScheduler()` - Returns reference to internal scheduler (for adding app tasks)
- `getTransitionCount()` - Returns number of transitions since start (for diagnostics)
- `forceTransitionTo(state)` - Bypass transition table (for fault recovery)

### smTransition

Defines a state transition as a tuple:

```cpp
struct smTransition {
    smState* fromState;      // Source state
    uint8_t exitCondition;   // Exit code that triggers transition
    smState* toState;        // Destination state
};
```

## Exit Codes

Built-in exit codes defined in `smAction.h`:

| Code | Value | Meaning |
|------|-------|---------|
| `EXIT_NONE` | 0 | No exit requested |
| `EXIT_COMPLETE` | 1 | Normal completion |
| `EXIT_TIMEOUT` | 2 | Task timed out |
| `EXIT_ERROR` | 3 | Error occurred |
| `EXIT_CANCEL` | 4 | Cancelled by request |
| `EXIT_ABORT` | 5 | Aborted (emergency) |
| `EXIT_USER` | 16 | Start of user-defined codes |

Application-specific exit codes should start at `EXIT_USER`:

```cpp
#define EXIT_BTN_PRESS    (EXIT_USER + 0)
#define EXIT_BTN_HOLD     (EXIT_USER + 1)
#define EXIT_SENSOR_TRIP  (EXIT_USER + 2)
```

## Usage Example

### 1. Define Actions

```cpp
class IdleAction : public smAction {
public:
    IdleAction() : smAction(nullptr, "IDLE") {}

    void onEnter() override {
        Serial.println("Entering IDLE");
    }

    bool onRun() override {
        if (buttonPressed()) {
            requestExit(EXIT_BTN_PRESS);
        }
        return true;
    }

    void onExit() override {
        Serial.println("Leaving IDLE");
    }
};

class RunningAction : public smAction {
public:
    RunningAction() : smAction(nullptr, "RUNNING") {}

    void onEnter() override {
        mStartTime = millis();
        Serial.println("Entering RUNNING");
    }

    bool onRun() override {
        // Auto-complete after 5 seconds
        if (millis() - mStartTime > 5000) {
            requestExit(EXIT_COMPLETE);
        }
        return true;
    }

private:
    unsigned long mStartTime;
};
```

### 2. Create States and Transitions

```cpp
// Instantiate actions
IdleAction actionIdle;
RunningAction actionRunning;

// Create states
smState STATE_IDLE(&actionIdle, "IDLE");
smState STATE_RUNNING(&actionRunning, "RUNNING");

// Define state array
smState* states[] = {
    &STATE_IDLE,
    &STATE_RUNNING
};

// Define transitions
smTransition transitions[] = {
    { &STATE_IDLE,    EXIT_BTN_PRESS, &STATE_RUNNING },
    { &STATE_RUNNING, EXIT_COMPLETE,  &STATE_IDLE }
};

// Create machine
smMachine machine(states, 2, transitions, 2);
```

### 3. Initialize and Run

```cpp
void setup() {
    Serial.begin(115200);

    // Initialize all states
    machine.begin();

    // Start in IDLE state
    machine.start(&STATE_IDLE);
}

// NOTE: Do not define loop() - the library provides it automatically.
// The global _smMachineInstance is set during smMachine construction
// and the library's loop() calls machine.execute() for you.
```

## Loop Ownership

The library provides its own `loop()` implementation in `smMachine.cpp`:

```cpp
// Global pointer set during smMachine construction
extern smMachine* _smMachineInstance;

// Library-provided loop()
void loop() {
    if (_smMachineInstance) {
        _smMachineInstance->execute();
    }
}
```

This means:
- **Do not define `loop()` in your sketch** - the library handles it
- Only one `smMachine` instance is supported (the last one constructed becomes active)
- The machine executes automatically after `setup()` completes

## Transition Flow

```
┌──────────────────────────────────────────────────────────────┐
│  1. Action calls requestExit(exitCode)                       │
│                         ↓                                    │
│  2. Machine receives transition request                      │
│                         ↓                                    │
│  3. Machine searches transition table for                    │
│     {currentState, exitCode, ?}                              │
│                         ↓                                    │
│  ┌─────────────────────┴─────────────────────┐               │
│  │                                           │               │
│  ↓ Found                                     ↓ Not Found     │
│  4a. Call current action's onExit()          4b. Call        │
│  5a. Switch to new state                         onInvalid-  │
│  6a. Call new action's onEnter()                 Transition()│
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

## Advanced Features

### Adding Application Tasks

The machine exposes its internal scheduler so applications can add their own tasks to the execution chain:

```cpp
// Define an application task
Task taskHeartbeat(1000, TASK_FOREVER, []() {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
});

void setup() {
    machine.begin();

    // Add application task to the machine's scheduler
    machine.getScheduler().addTask(taskHeartbeat);
    taskHeartbeat.enable();

    machine.start(&STATE_IDLE);
}
```

This allows background tasks (heartbeat LEDs, sensor polling, watchdog feeds, etc.) to run alongside the state machine without defining a custom `loop()`.

### Force Transition

Bypass the transition table for fault recovery:

```cpp
machine.forceTransitionTo(&STATE_IDLE);
```

### Custom Invalid Transition Handling

Override at the action level:

```cpp
void MyAction::onInvalidTransition(uint8_t exitCode) {
    Serial.printf("No transition for exit code %d\n", exitCode);
}
```

Or at the machine level:

```cpp
class MyMachine : public smMachine {
    void onInvalidTransition(smState* fromState, uint8_t exitCode) override {
        Serial.printf("Invalid: %s -> code %d\n", fromState->getName(), exitCode);
    }
};
```

### Device Integration

Actions can own and manage hardware devices:

```cpp
class MotorDevice : public smDevice {
public:
    bool begin() override { /* init pins */ return true; }
    bool start() override { /* enable motor */ return true; }
    void stop() override { /* disable motor */ }
    void end() override { /* release pins */ }
};

class MotorAction : public smAction {
public:
    MotorAction(MotorDevice* motor) : smAction(motor, "MOTOR") {}

    void onEnter() override {
        mDevice->start();  // Start motor when state entered
    }

    void onExit() override {
        mDevice->stop();   // Stop motor when state exited
    }
};
```

## Best Practices

### Non-Blocking Code

Actions must not block. Use non-blocking patterns:

```cpp
// BAD - blocks execution
void onRun() {
    delay(1000);
    requestExit(EXIT_COMPLETE);
}

// GOOD - non-blocking check
void onEnter() {
    mStartTime = millis();
}

bool onRun() {
    if (millis() - mStartTime > 1000) {
        requestExit(EXIT_COMPLETE);
    }
    return true;
}
```

### State Naming

Use descriptive state names for debugging:

```cpp
smState STATE_MOTOR_EXTENDING(&actionExtend, "MOTOR_EXTENDING");
smState STATE_WAITING_INPUT(&actionWait, "WAITING_INPUT");
```

### Exit Code Organization

Group related exit codes in a header file:

```cpp
// exitcodes.h
#pragma once
#include <smAction.h>

// Button events (16-31)
#define EXIT_BTN_UP       (EXIT_USER + 0)
#define EXIT_BTN_DOWN     (EXIT_USER + 1)
#define EXIT_BTN_LEFT     (EXIT_USER + 2)
#define EXIT_BTN_RIGHT    (EXIT_USER + 3)

// Sensor events (32-47)
#define EXIT_SENSOR_TRIP  (EXIT_USER + 16)
#define EXIT_LIMIT_HIT    (EXIT_USER + 17)

// Fault conditions (48-63)
#define EXIT_TILT_FAULT   (EXIT_USER + 32)
#define EXIT_TEMP_FAULT   (EXIT_USER + 33)
```

## File Reference

| File | Purpose |
|------|---------|
| `StateMachine.h` | Convenience header (includes all components) |
| `smMachine.h/cpp` | State machine orchestrator |
| `smAction.h/cpp` | Base action class with lifecycle hooks |
| `smState.h/cpp` | State wrapper with TaskScheduler integration |
| `smDevice.h` | Device interface for hardware abstraction |

## License

This library is provided as-is for use in embedded projects.

## Author

Anatoli Arkhipenko (arkhipenko@hotmail.com)

---
[![github](https://github.com/arkhipenko/resources/blob/master/smart4smart_hero_banner.gif)](https://smart4smart.com/)
