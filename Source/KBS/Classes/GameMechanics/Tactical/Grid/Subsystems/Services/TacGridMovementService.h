// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/GridCoordinates.h"
#include "TacGridMovementService.generated.h"


class AUnit;
class UBattleTeam;
class UGridDataManager;
enum class ETeamSide : uint8;
enum class ETacGridLayer : uint8;

USTRUCT()
struct FTacMovementSegment
{
	GENERATED_BODY()

	FVector Start;
	FVector End;
	float Duration;  // Distance / Speed, pre-calculated
	FRotator TargetRotation;  // Face this direction during/after segment

	FTacMovementSegment() = default;
	FTacMovementSegment(FVector InStart, FVector InEnd, float InDuration, FRotator InRotation)
		: Start(InStart), End(InEnd), Duration(InDuration), TargetRotation(InRotation) {
	}
};

USTRUCT()
struct FTacMovementVisualData
{
	GENERATED_BODY()

	// Movement queue
	TArray<FTacMovementSegment> Segments;
	float CurrentSegmentProgress = 0.0f;  // Time elapsed in current segment

	// Rotation state
	FRotator CurrentRotation;
	FRotator TargetRotation;
	float RotationProgress = 1.0f;  // 1.0 = rotation complete, <1.0 = rotating
	float RotationDuration = 0.15f;  // How long rotations take (configurable)

	// Final state
	FIntPoint FinalCell;
	ETeamSide UnitTeamSide;
	bool bApplyDefaultRotationAtEnd = true;
	bool bApplyFlankRotationAtEnd = false;
};



UCLASS()
class KBS_API UTacGridMovementService : public UObject
{
	GENERATED_BODY()
public:
	UTacGridMovementService();
	void Initialize(UGridDataManager* InDataManager);

	// Grid metadata initialization (call after initial PlaceUnit)
	void InitializeUnitGridMetadata(AUnit* Unit, FTacCoordinates Position);

	// moves unit, generating it's visuals
	bool MoveUnit(AUnit* Unit, FTacCoordinates Where, FTacMovementVisualData& OutVisuals, TOptional<FTacMovementVisualData>& OutSwappedUnitVisuals);
	// make visuals without actual movement
	FTacMovementVisualData MakeMovementVisual(AUnit* UnitToMove, FTacCoordinates Where);
	TArray<FTacMovementSegment> MakeUnitPath(AUnit* UnitToMove, FTacCoordinates Where);

	// Instant movement (game mechanic): Use for abilities, teleport effects
	// Currently identical to PushUnitToCell, reserved for future VFX/sound
	bool TeleportUnit(AUnit* UnitToMove, FTacCoordinates Where);

	// Low-level data operation: Use for internal grid manipulation only
	// Purely updates grid state without game logic implications
	bool PushUnitToCell(AUnit* UnitToMove, FTacCoordinates Where);
private:
	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;

	// Validation helpers
	bool ValidateMovementParameters(AUnit* Unit, FTacCoordinates Where) const;
	bool GetUnitCurrentPosition(AUnit* Unit, FTacCoordinates& OutPos) const;
	bool ValidateAndGetPosition(AUnit* Unit, FTacCoordinates Where, FTacCoordinates& OutCurrentPos) const;

	// Movement execution helpers
	bool ExecuteDataMove(AUnit* Unit, FTacCoordinates From, FTacCoordinates To);
	bool ExecuteSwapMove(AUnit* Unit1, FTacCoordinates Pos1, AUnit* Unit2, FTacCoordinates Pos2);
	FTacMovementSegment CreateMovementSegment(FVector Start, FVector End, float Speed, FRotator TargetRotation);


	// Visual generation helpers
	FTacMovementVisualData BuildVisualData(const TArray<FTacMovementSegment>& Segments,
		FTacCoordinates FinalCell, ETeamSide TeamSide, bool bIsFlankCell);

	// Orientation calculation
	FRotator CalculateCellOrientation(FTacCoordinates Location, ETeamSide TeamSide) const;

	// Grid metadata management
	void UpdateUnitGridMetadata(AUnit* Unit, FTacCoordinates Position, bool bOnField);
};