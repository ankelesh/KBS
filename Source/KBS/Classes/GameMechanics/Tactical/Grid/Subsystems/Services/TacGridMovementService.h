// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/TacMovementTypes.h"
#include "TacGridMovementService.generated.h"


class AUnit;
class UBattleTeam;
class UGridDataManager;
enum class ETeamSide : uint8;
enum class ETacGridLayer : uint8;



UCLASS()
class KBS_API UTacGridMovementService : public UObject
{
	GENERATED_BODY()
public:
	UTacGridMovementService();
	void Initialize(UGridDataManager* InDataManager);

	// Moves unit in grid, builds path, notifies unit (which broadcasts to listeners including visuals)
	bool MoveUnit(AUnit* Unit, FTacCoordinates Where);
	// make visuals without actual movement
	FTacMovementVisualData MakeMovementVisual(AUnit* UnitToMove, FTacCoordinates Where);
	TArray<FTacMovementSegment> MakeUnitPath(AUnit* UnitToMove, FTacCoordinates Where);

	// Instant movement (emits, does not lend visual pathing)
	bool TeleportUnit(AUnit* UnitToMove, FTacCoordinates Where);

	// Non-emitting non-visualized movement
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
	TArray<FTacMovementSegment> BuildPathSegments(AUnit* Unit, FTacCoordinates Where);
	FTacMovementVisualData BuildVisualData(const TArray<FTacMovementSegment>& Segments,
		FTacCoordinates FinalCell, ETeamSide TeamSide, bool bIsFlankCell, bool bIsMultiCell);

	// Orientation calculation
	FRotator CalculateCellOrientation(ETeamSide TeamSide) const;

};