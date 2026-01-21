#pragma once

#include <Arduino.h>

typedef enum {
    smON,
    smOFF,
    smSTARTING,
    smSTOPPING
} smDeviceState_t;

class smDevice {
public:
    smDevice(const char* name = "DEVICE") : mName(name), mState(smOFF) {}
    virtual ~smDevice() {}

    virtual bool begin() = 0;   // Initialize hardware
    virtual bool start() = 0;   // Turn on / activate
    virtual void stop() = 0;    // Turn off / deactivate
    virtual void end() = 0;     // Release hardware

    // State accessors
    smDeviceState_t getState() { return mState; }

    // Name for logging/identification
    const char* getName() const { return mName; }
    void setName(const char* name) { mName = name; }

protected:
    void setState(smDeviceState_t state) { mState = state; }

    const char* mName;
    smDeviceState_t mState;
};
