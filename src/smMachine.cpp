#include "smMachine.h"

smMachine* _smMachineInstance = nullptr;

smMachine::smMachine(smState* aStates[], uint8_t aNumStates,
                     smTransition* aTransitions, uint8_t aNumTransitions)
    : mStates(aStates)
    , mTransitions(aTransitions)
    , mNumStates(aNumStates)
    , mNumTransitions(aNumTransitions)
    , mCurrentState(nullptr)
    , mPreviousState(nullptr)
    , mRunning(false)
    , mTransitionCount(0)
{
    _smMachineInstance = this;
}

bool smMachine::begin() {
    bool ok = true;

    for (uint8_t i = 0; i < mNumStates; i++) {
        if (mStates[i]) {
            mStates[i]->setMachine(this);
            if (mStates[i]->getAction()) {
                mStates[i]->getAction()->setMachine(this);
            }
            mScheduler.addTask(*mStates[i]);
            ok &= mStates[i]->begin();
        }
    }

    return ok;
}

bool smMachine::start(smState* initialState) {
    if (initialState) {
        mCurrentState = initialState;
        mCurrentState->enable();
        mRunning = true;
        return true;
    }
    return false;
}

void smMachine::stop() {
    if (mCurrentState) {
        mCurrentState->disable();
    }
    mRunning = false;
}

void smMachine::execute() {
    if (mRunning) {
        mScheduler.execute();
    }
}

void smMachine::requestTransition(uint8_t exitCode) {
    smState* nextState = findNextState(mCurrentState, exitCode);

    if (nextState) {
        transitionTo(nextState);
    } else {
        if (mCurrentState && mCurrentState->getAction()) {
            mCurrentState->getAction()->onInvalidTransition(exitCode);
        } else {
            onInvalidTransition(mCurrentState, exitCode);
        }
    }
}

smState* smMachine::findNextState(smState* fromState, uint8_t exitCode) {
    for (uint8_t i = 0; i < mNumTransitions; i++) {
        if (mTransitions[i].fromState == fromState &&
            mTransitions[i].exitCondition == exitCode) {
            return mTransitions[i].toState;
        }
    }
    return nullptr;
}

void smMachine::transitionTo(smState* toState) {
    if (!toState) {
        onInvalidTransition(mCurrentState, 0);
        return;
    }

    // Track previous state
    mPreviousState = mCurrentState;

    // Disable current state (triggers onExit)
    if (mCurrentState) {
        mCurrentState->disable();
    }

    // Enable new state (triggers onEnter)
    mCurrentState = toState;
    mCurrentState->enable();

    // Increment transition counter
    mTransitionCount++;
}

void smMachine::forceTransitionTo(smState* toState) {
    transitionTo(toState);
}

void smMachine::onInvalidTransition(smState* fromState, uint8_t exitCode) {
    mRunning = false;
}

// Arduino loop() implementation
void loop() {
    if (_smMachineInstance) {
        _smMachineInstance->execute();
    }
}
