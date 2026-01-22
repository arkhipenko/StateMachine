#include "Button.h"

Button::Button(uint8_t aPin, bool aActiveLow, bool aUsePullup)
    : mButton(aPin, aActiveLow, aUsePullup)
    , mClickedFlag(false)
    , mLongPressedFlag(false)
    , mDoubleClickedFlag(false)
{
}

Button::~Button() {
}

bool Button::begin() {
    mButton.attachClick(onClickStatic, this);
    mButton.attachLongPressStop(onLongPressStatic, this);
    mButton.attachDoubleClick(onDoubleClickStatic, this);
    mState = smOFF;
    return true;
}

bool Button::start() {
    mClickedFlag = false;
    mLongPressedFlag = false;
    mDoubleClickedFlag = false;
    mState = smON;
    return true;
}

void Button::stop() {
    mState = smOFF;
}

void Button::end() {
    stop();
}

void Button::tick() {
    if (mState == smON) {
        mButton.tick();
    }
}

bool Button::wasPressed() {
    if (mClickedFlag) {
        mClickedFlag = false;
        return true;
    }
    return false;
}

bool Button::wasLongPressed() {
    if (mLongPressedFlag) {
        mLongPressedFlag = false;
        return true;
    }
    return false;
}

bool Button::wasDoubleClicked() {
    if (mDoubleClickedFlag) {
        mDoubleClickedFlag = false;
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
    mClickedFlag = true;
}

void Button::onLongPress() {
    mLongPressedFlag = true;
}

void Button::onDoubleClick() {
    mDoubleClickedFlag = true;
}
