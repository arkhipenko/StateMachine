#pragma once

// Ensure OO callbacks are enabled for TaskScheduler
#ifndef _TASK_OO_CALLBACKS
#define _TASK_OO_CALLBACKS
#endif

#include <TaskSchedulerDeclarations.h>
#include "smAction.h"

// Default state execution interval (milliseconds)
#define SM_DEFAULT_INTERVAL_MS  1

// Forward declaration
class smMachine;

class smState : public Task {
public:
    smState(smAction* aAction,
            const char* name = "UNNAMED",
            unsigned long aInterval = SM_DEFAULT_INTERVAL_MS,
            long aIterations = TASK_FOREVER);

    void setMachine(smMachine* machine) { mMachine = machine; }
    void setName(const char* name) { mName = name; }
    smAction* getAction() { return mAction; }
    const char* getName() const { return mName; }

    // Time tracking
    unsigned long getEnterTime() { return mEnterTime; }

    // Lifecycle
    bool begin();
    void end();

    // Task callbacks (OO style)
    bool OnEnable() override;
    bool Callback() override;
    void OnDisable() override;

private:
    smAction* mAction;
    smMachine* mMachine;
    const char* mName;
    unsigned long mEnterTime;
};
