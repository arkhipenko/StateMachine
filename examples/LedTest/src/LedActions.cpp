#include "LedActions.h"
#include <ArduinoLog.h>

// === LedButtonAction (base) ===

LedButtonAction::LedButtonAction(LED* aLed, Button* aButton)
    : smAction(aLed)
    , mLed(aLed)
    , mButton(aButton)
{
    Log.trace("LedButtonAction::LedButtonAction()" CR);
}

bool LedButtonAction::begin() {
    Log.trace("LedButtonAction::begin() enter" CR);
    bool ok = true;
    if (mLed) ok &= mLed->begin();
    if (mButton) ok &= mButton->begin();
    Log.trace("LedButtonAction::begin() exit ok=%T" CR, ok);
    return ok;
}

void LedButtonAction::end() {
    Log.trace("LedButtonAction::end() enter" CR);
    if (mLed) mLed->end();
    if (mButton) mButton->end();
    Log.trace("LedButtonAction::end() exit" CR);
}

bool LedButtonAction::checkButton() {
    if (mButton) {
        mButton->tick();
        if (mButton->wasPressed()) {
            Log.trace("LedButtonAction::checkButton() button pressed" CR);
            requestExit(EXIT_BUTTON_PRESS);
            return true;
        }
    }
    return false;
}

// === LedOffAction ===

LedOffAction::LedOffAction(LED* aLed, Button* aButton)
    : LedButtonAction(aLed, aButton)
{
    Log.trace("LedOffAction::LedOffAction()" CR);
}

void LedOffAction::onEnter() {
    Log.trace("LedOffAction::onEnter()" CR);
    if (mLed) mLed->off();
    if (mButton) mButton->start();
}

bool LedOffAction::onRun() {
    checkButton();
    return true;  // Keep running until exit requested
}

void LedOffAction::onExit() {
    Log.trace("LedOffAction::onExit()" CR);
    if (mButton) mButton->stop();
}

// === LedOnAction ===

LedOnAction::LedOnAction(LED* aLed, Button* aButton)
    : LedButtonAction(aLed, aButton)
{
    Log.trace("LedOnAction::LedOnAction()" CR);
}

void LedOnAction::onEnter() {
    Log.trace("LedOnAction::onEnter()" CR);
    if (mLed) mLed->on();
    if (mButton) mButton->start();
}

bool LedOnAction::onRun() {
    checkButton();
    return true;
}

void LedOnAction::onExit() {
    Log.trace("LedOnAction::onExit()" CR);
    if (mButton) mButton->stop();
}

// === LedBlinkAction ===

LedBlinkAction::LedBlinkAction(LED* aLed, Button* aButton, uint32_t aIntervalMs)
    : LedButtonAction(aLed, aButton)
    , mIntervalMs(aIntervalMs)
    , mLastToggleTime(0)
{
    Log.trace("LedBlinkAction::LedBlinkAction() interval=%l" CR, aIntervalMs);
}

void LedBlinkAction::onEnter() {
    Log.trace("LedBlinkAction::onEnter() interval=%l" CR, mIntervalMs);
    if (mLed) mLed->on();
    if (mButton) mButton->start();
    mLastToggleTime = millis();
}

bool LedBlinkAction::onRun() {
    // Blink LED
    if ((millis() - mLastToggleTime) >= mIntervalMs) {
        if (mLed) mLed->toggle();
        mLastToggleTime = millis();
    }

    checkButton();
    return true;
}

void LedBlinkAction::onExit() {
    Log.trace("LedBlinkAction::onExit()" CR);
    if (mButton) mButton->stop();
    if (mLed) mLed->off();
}
