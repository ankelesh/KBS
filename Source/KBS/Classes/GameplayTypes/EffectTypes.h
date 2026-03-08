#pragma once
#include "CoreMinimal.h"


UENUM(BlueprintType)
enum class EEffectPolarity : uint8
{
	Positive UMETA(DisplayName = "Positive"),
	Negative UMETA(DisplayName = "Negative"),
	Neutral UMETA(DisplayName = "Neutral"),
};

UENUM(BlueprintType)
enum class EEffectStackPolicy : uint8
{
	// rejects any reapplication
	Unique UMETA(DisplayName = "Unique"),
	// replaces without asking effect
	AlwaysReplaced UMETA(DisplayName = "AlwaysReplaced"),
	// refresh old as-is without asking
	RefreshOld UMETA(DisplayName = "RefreshOld"),
	// refresh or replace to better one, ask effect for comparison
	RefreshOrReplace UMETA(DisplayName = "RefreshOrReplace"),
	// stack as many as possible, no asking
	StackInfinite UMETA(DisplayName = "StaskInfinite"),
	// check stack limit of effect
	Stack UMETA(DisplayName = "Stack"),
	// ask effect directly
	Custom UMETA(DisplayName = "Custom"),
};

UENUM(BlueprintType)
enum class EReapplyDecision : uint8
{
	// leave old
	Old UMETA(DisplayName = "Old"),
	// replace with new
	New UMETA(DisplayName = "New"),
	// effectively, prolong old
	OverrideDuration UMETA(DisplayName = "OverrideDuration"),
	// inner logic processing handled everything, skip
	DoNothing UMETA(DisplayName = "DoNothing"),
};

