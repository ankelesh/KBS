#pragma once
#include "CoreMinimal.h"
#include "TacMovementTypes.generated.h"

enum class ETeamSide : uint8;

USTRUCT()
struct FTacMovementSegment
{
	GENERATED_BODY()

	FVector Start;
	FVector End;
	float Duration;  // Distance / Speed, pre-calculated
	FRotator TargetRotation;  // Face this direction during segment

	FTacMovementSegment() = default;
	FTacMovementSegment(FVector InStart, FVector InEnd, float InDuration, FRotator InRotation)
		: Start(InStart), End(InEnd), Duration(InDuration), TargetRotation(InRotation) {}
};

USTRUCT()
struct FTacMovementVisualData
{
	GENERATED_BODY()

	TArray<FTacMovementSegment> Segments;
	float CurrentSegmentProgress = 0.0f;

	FRotator TargetRotation;  // Final cell orientation (set for non-flank cells)
	bool bApplyDefaultRotationAtEnd = true;
	bool bApplyFlankRotationAtEnd = false;
};
