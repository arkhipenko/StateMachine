#include "smAction.h"
#include "smMachine.h"

void smAction::requestExit(uint8_t exitCode) {
    mExitCode = exitCode;
    if (mMachine) {
        mMachine->requestTransition(exitCode);
    }
}
