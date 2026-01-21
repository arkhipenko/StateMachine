#pragma once

#include "smDevice.h"
#include <TinyPICO.h>

class LED : public smDevice {
public:
    LED(uint8_t r = 0, uint8_t g = 255, uint8_t b = 0);  // Default green
    ~LED();

    // Lifecycle
    bool begin() override;
    bool start() override;  // Enter idle state (LED off)
    void stop() override;   // Enter final state (LED off)
    void end() override;

    // Device operations
    void on();
    void off();
    void toggle();
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void setBrightness(uint8_t brightness);
    bool isOn() { return mLedOn; }

private:
    TinyPICO mTinyPICO;
    uint8_t mR, mG, mB;
    uint8_t mBrightness;
    bool mLedOn;
};
