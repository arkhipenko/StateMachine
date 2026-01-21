#pragma once

#include <TaskSchedulerDeclarations.h>
#include "smState.h"

struct smTransition {
    smState* fromState;
    uint8_t exitCondition;
    smState* toState;
};

class smMachine {
public:
    smMachine(smState* aStates[], uint8_t aNumStates,
              smTransition* aTransitions, uint8_t aNumTransitions);

    bool begin();
    bool start(smState* initialState);
    void stop();
    void execute();

    // Request transition from current state with exit code
    void requestTransition(uint8_t exitCode);

    // State accessors
    smState* getCurrentState() { return mCurrentState; }
    smState* getPreviousState() { return mPreviousState; }

    // Running state
    bool isRunning() { return mRunning; }

    // Get scheduler reference for adding application tasks
    Scheduler& getScheduler() { return mScheduler; }

    // Transition counter for diagnostics
    unsigned long getTransitionCount() { return mTransitionCount; }

    // Force transition to specific state (for fault recovery, etc.)
    void forceTransitionTo(smState* toState);

    // Called when transition is invalid and action has no handler
    virtual void onInvalidTransition(smState* fromState, uint8_t exitCode);

private:
    void transitionTo(smState* toState);
    smState* findNextState(smState* fromState, uint8_t exitCode);

    Scheduler mScheduler;
    smState** mStates;
    smTransition* mTransitions;
    uint8_t mNumStates;
    uint8_t mNumTransitions;
    smState* mCurrentState;
    smState* mPreviousState;
    bool mRunning;
    unsigned long mTransitionCount;
};

// Global machine pointer for loop()
extern smMachine* _smMachineInstance;
