#include "LED.h"
#include <ArduinoLog.h>

LED::LED(uint8_t r, uint8_t g, uint8_t b)
    : mR(r)
    , mG(g)
    , mB(b)
    , mBrightness(128)
    , mLedOn(false)
{
    Log.trace("LED::LED() r=%d g=%d b=%d" CR, r, g, b);
}

LED::~LED() {
    Log.trace("LED::~LED()" CR);
    end();
}

bool LED::begin() {
    Log.trace("LED::begin() enter" CR);
    off();
    Log.trace("LED::begin() exit" CR);
    return true;
}

bool LED::start() {
    Log.trace("LED::start() enter" CR);
    off();
    mState = smON;
    Log.trace("LED::start() exit" CR);
    return true;
}

void LED::stop() {
    Log.trace("LED::stop() enter" CR);
    off();
    mState = smOFF;
    Log.trace("LED::stop() exit" CR);
}

void LED::end() {
    Log.trace("LED::end() enter" CR);
    off();
    Log.trace("LED::end() exit" CR);
}

void LED::on() {
    Log.trace("LED::on()" CR);
    mTinyPICO.DotStar_SetPixelColor(mR, mG, mB);
    mLedOn = true;
}

void LED::off() {
    Log.trace("LED::off()" CR);
    mTinyPICO.DotStar_SetPixelColor(0, 0, 0);
    mLedOn = false;
}

void LED::toggle() {
    Log.trace("LED::toggle() wasOn=%T" CR, mLedOn);
    if (mLedOn) {
        off();
    } else {
        on();
    }
}

void LED::setColor(uint8_t r, uint8_t g, uint8_t b) {
    Log.trace("LED::setColor() r=%d g=%d b=%d" CR, r, g, b);
    mR = r;
    mG = g;
    mB = b;
    if (mLedOn) {
        on();  // Update color if LED is on
    }
}

void LED::setBrightness(uint8_t brightness) {
    Log.trace("LED::setBrightness() brightness=%d" CR, brightness);
    mBrightness = brightness;
    mTinyPICO.DotStar_SetBrightness(brightness);
}
