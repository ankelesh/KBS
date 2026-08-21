#pragma once
#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
#include "HAL/IConsoleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogKBSPresentation, Log, All)

// Non-zero: log each assembled action (index, class, attacker, tag) plus montage resolution failures.
extern TAutoConsoleVariable<int32> CVarDumpSequence;
