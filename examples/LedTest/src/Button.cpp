#include "Button.h"
#include <ArduinoLog.h>

Button::Button(uint8_t aPin, bool aActiveLow, bool aUsePullup)
    : mButton(aPin, aActiveLow, aUsePullup)
    , mClickedFlag(false)
    , mLongPressedFlag(false)
    , mDoubleClickedFlag(false)
{
    Log.trace("Button::Button() pin=%d activeLow=%T pullup=%T" CR, aPin, aActiveLow, aUsePullup);
}

Button::~Button() {
    Log.trace("Button::~Button()" CR);
}

bool Button::begin() {
    Log.trace("Button::begin() enter" CR);
    mButton.attachClick(onClickStatic, this);
    mButton.attachLongPressStop(onLongPressStatic, this);
    mButton.attachDoubleClick(onDoubleClickStatic, this);
    mState = smOFF;
    Log.trace("Button::begin() exit" CR);
    return true;
}

bool Button::start() {
    Log.trace("Button::start() enter" CR);
    mClickedFlag = false;
    mLongPressedFlag = false;
    mDoubleClickedFlag = false;
    mState = smON;
    Log.trace("Button::start() exit" CR);
    return true;
}

void Button::stop() {
    Log.trace("Button::stop() enter" CR);
    mState = smOFF;
    Log.trace("Button::stop() exit" CR);
}

void Button::end() {
    Log.trace("Button::end() enter" CR);
    stop();
    Log.trace("Button::end() exit" CR);
}

void Button::tick() {
    if (mState == smON) {
        mButton.tick();
    }
}

bool Button::wasPressed() {
    if (mClickedFlag) {
        mClickedFlag = false;
        Log.trace("Button::wasPressed() returning true" CR);
        return true;
    }
    return false;
}

bool Button::wasLongPressed() {
    if (mLongPressedFlag) {
        mLongPressedFlag = false;
        Log.trace("Button::wasLongPressed() returning true" CR);
        return true;
    }
    return false;
}

bool Button::wasDoubleClicked() {
    if (mDoubleClickedFlag) {
        mDoubleClickedFlag = false;
        Log.trace("Button::wasDoubleClicked() returning true" CR);
        return true;
    }
    return false;
}

void Button::onClickStatic(void* ptr) {
    static_cast<Button*>(ptr)->onClick();
}

void Button::onLongPressStatic(void* ptr) {
    static_cast<Button*>(ptr)->onLongPress();
}

void Button::onDoubleClickStatic(void* ptr) {
    static_cast<Button*>(ptr)->onDoubleClick();
}

void Button::onClick() {
    Log.trace("Button::onClick()" CR);
    mClickedFlag = true;
}

void Button::onLongPress() {
    Log.trace("Button::onLongPress()" CR);
    mLongPressedFlag = true;
}

void Button::onDoubleClick() {
    Log.trace("Button::onDoubleClick()" CR);
    mDoubleClickedFlag = true;
}
