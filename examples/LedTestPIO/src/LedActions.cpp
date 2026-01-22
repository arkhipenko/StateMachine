#include <Arduino.h>
#include "LedActions.h"

// Helper to convert exit codes to readable names
static const char* exitCodeName(uint8_t code) {
    switch (code) {
        case EXIT_NONE:         return "NONE";
        case EXIT_COMPLETE:     return "COMPLETE";
        case EXIT_TIMEOUT:      return "TIMEOUT";
        case EXIT_ERROR:        return "ERROR";
        case EXIT_CANCEL:       return "CANCEL";
        case EXIT_ABORT:        return "ABORT";
        case EXIT_BUTTON_PRESS: return "BUTTON_PRESS";
        default:                return "UNKNOWN";
    }
}

// === LedButtonAction (base) ===

LedButtonAction::LedButtonAction(LED* aLed, Button* aButton)
    : smAction(aLed)
    , mLed(aLed)
    , mButton(aButton)
{
}

bool LedButtonAction::begin() {
    bool ok = true;
    if (mLed) ok &= mLed->begin();
    if (mButton) ok &= mButton->begin();
    return ok;
}

void LedButtonAction::end() {
    if (mLed) mLed->end();
    if (mButton) mButton->end();
}

bool LedButtonAction::checkButton() {
    if (mButton) {
        mButton->tick();
        if (mButton->wasPressed()) {
            Serial.println("  [Button pressed - requesting exit]");
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
}

void LedOffAction::onEnter() {
    Serial.println(">> Entering STATE_OFF");
    if (mLed) mLed->off();
    if (mButton) mButton->start();
}

bool LedOffAction::onRun() {
    checkButton();
    return true;  // Keep running until exit requested
}

void LedOffAction::onExit() {
    Serial.print("<< Exiting STATE_OFF (");
    Serial.print(exitCodeName(getExitCode()));
    Serial.println(")");
    if (mButton) mButton->stop();
}

// === LedOnAction ===

LedOnAction::LedOnAction(LED* aLed, Button* aButton)
    : LedButtonAction(aLed, aButton)
{
}

void LedOnAction::onEnter() {
    Serial.println(">> Entering STATE_ON (timeout=5s)");
    if (mLed) mLed->on();
    if (mButton) mButton->start();
}

bool LedOnAction::onRun() {
    checkButton();
    return true;
}

void LedOnAction::onExit() {
    Serial.print("<< Exiting STATE_ON (");
    Serial.print(exitCodeName(getExitCode()));
    Serial.println(")");
    if (mButton) mButton->stop();
}

// === LedBlinkAction ===

LedBlinkAction::LedBlinkAction(LED* aLed, Button* aButton, uint32_t aIntervalMs)
    : LedButtonAction(aLed, aButton)
    , mIntervalMs(aIntervalMs)
    , mLastToggleTime(0)
{
}

void LedBlinkAction::onEnter() {
    Serial.print(">> Entering STATE_BLINK (interval=");
    Serial.print(mIntervalMs);
    Serial.println("ms)");
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
    Serial.print("<< Exiting STATE_BLINK (");
    Serial.print(exitCodeName(getExitCode()));
    Serial.println(")");
    if (mButton) mButton->stop();
    if (mLed) mLed->off();
}
