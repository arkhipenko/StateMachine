#pragma once

#include "smAction.h"
#include "LED.h"
#include "Button.h"

// Exit condition for button press
#define EXIT_BUTTON_PRESS EXIT_USER

// Base class for LED actions that respond to button
class LedButtonAction : public smAction {
public:
    LedButtonAction(LED* aLed, Button* aButton);

    bool begin() override;
    void end() override;

protected:
    LED* mLed;
    Button* mButton;

    // Check button and request exit if pressed
    bool checkButton();
};

// LED Off - keeps LED off, exits on button press
class LedOffAction : public LedButtonAction {
public:
    LedOffAction(LED* aLed, Button* aButton);

    void onEnter() override;
    bool onRun() override;
    void onExit() override;
};

// LED On - keeps LED on, exits on button press
class LedOnAction : public LedButtonAction {
public:
    LedOnAction(LED* aLed, Button* aButton);

    void onEnter() override;
    bool onRun() override;
    void onExit() override;
};

// LED Blink - blinks LED at specified interval, exits on button press
class LedBlinkAction : public LedButtonAction {
public:
    LedBlinkAction(LED* aLed, Button* aButton, uint32_t aIntervalMs);

    void onEnter() override;
    bool onRun() override;
    void onExit() override;

private:
    uint32_t mIntervalMs;
    uint32_t mLastToggleTime;
};
