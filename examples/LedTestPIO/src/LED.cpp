#include "LED.h"

LED::LED(uint8_t r, uint8_t g, uint8_t b)
    : mR(r)
    , mG(g)
    , mB(b)
    , mBrightness(128)
    , mLedOn(false)
{
}

LED::~LED() {
    end();
}

bool LED::begin() {
    off();
    return true;
}

bool LED::start() {
    off();
    mState = smON;
    return true;
}

void LED::stop() {
    off();
    mState = smOFF;
}

void LED::end() {
    off();
}

void LED::on() {
    mTinyPICO.DotStar_SetPixelColor(mR, mG, mB);
    mLedOn = true;
}

void LED::off() {
    mTinyPICO.DotStar_SetPixelColor(0, 0, 0);
    mLedOn = false;
}

void LED::toggle() {
    if (mLedOn) {
        off();
    } else {
        on();
    }
}

void LED::setColor(uint8_t r, uint8_t g, uint8_t b) {
    mR = r;
    mG = g;
    mB = b;
    if (mLedOn) {
        on();  // Update color if LED is on
    }
}

void LED::setBrightness(uint8_t brightness) {
    mBrightness = brightness;
    mTinyPICO.DotStar_SetBrightness(brightness);
}
