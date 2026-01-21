#pragma once

// =============================================================================
// StateMachine.h - Convenience header for SM framework
// =============================================================================
// Include this single file to get all SM framework components:
//   - smDevice: Base class for hardware abstraction
//   - smAction: Base class for state behavior
//   - smState: State wrapper around actions
//   - smMachine: State machine orchestrator
// =============================================================================

#include "smDevice.h"
#include "smAction.h"
#include "smState.h"
#include "smMachine.h"
