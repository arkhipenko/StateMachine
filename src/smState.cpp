#include "smState.h"
#include "smMachine.h"

smState::smState(smAction* aAction, const char* name, unsigned long aInterval, long aIterations)
    : Task(aInterval, aIterations, nullptr, false)
    , mAction(aAction)
    , mMachine(nullptr)
    , mName(name)
    , mEnterTime(0)
{
}

bool smState::begin() {
    bool ok = true;
    if (mAction) {
        ok = mAction->begin();
    }
    return ok;
}

void smState::end() {
    if (mAction) {
        mAction->end();
    }
}

bool smState::OnEnable() {
    mEnterTime = millis();
    if (mAction) {
        mAction->resetExitCode();
        mAction->onEnter();
    }
    return true;
}

bool smState::Callback() {
    if (mAction) {
        return mAction->onRun();
    }
    return false;
}

void smState::OnDisable() {
    if (mAction) {
        mAction->onExit();
    }
    // If disabled due to timeout (and no explicit exit was requested),
    // request transition with EXIT_TIMEOUT
    if (timedOut() && mMachine && mAction && mAction->getExitCode() == EXIT_NONE) {
        mMachine->requestTransition(EXIT_TIMEOUT);
    }
}
