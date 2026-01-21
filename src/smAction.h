#pragma once

#include "smDevice.h"

// Forward declaration
class smMachine;

// Exit condition codes
#define EXIT_NONE       0
#define EXIT_COMPLETE   1   // Normal completion (iterations done)
#define EXIT_TIMEOUT    2   // Task timed out (auto-detected)
#define EXIT_ERROR      3   // Error occurred
#define EXIT_CANCEL     4   // Cancelled by request
#define EXIT_ABORT      5   // Aborted (emergency stop)
// User-defined exit conditions start at 16
#define EXIT_USER       16

class smAction {
public:
    smAction(smDevice* aDevice, const char* name = "ACTION")
        : mName(name), mDevice(aDevice), mMachine(nullptr), mExitCode(EXIT_NONE) {}
    virtual ~smAction() {}

    // Lifecycle
    virtual bool begin() { return mDevice ? mDevice->begin() : true; }
    virtual void end() { if (mDevice) mDevice->end(); }

    // Called when action becomes active (state entered)
    virtual void onEnter() {}

    // Called repeatedly while action is active
    // Return false to signal exit with current exit code
    virtual bool onRun() = 0;

    // Called when action becomes inactive (state exited)
    virtual void onExit() {}

    // Called when action signals an exit but transition is invalid
    virtual void onInvalidTransition(uint8_t exitCode) {}

    // Signal exit with condition code
    void requestExit(uint8_t exitCode);

    // Exit code accessors
    uint8_t getExitCode() { return mExitCode; }
    void resetExitCode() { mExitCode = EXIT_NONE; }

    // Machine accessors
    void setMachine(smMachine* machine) { mMachine = machine; }
    smMachine* getMachine() { return mMachine; }

    // Name for logging/identification
    const char* getName() const { return mName; }
    void setName(const char* name) { mName = name; }

protected:
    const char* mName;
    smDevice* mDevice;
    smMachine* mMachine;
    uint8_t mExitCode;
};
