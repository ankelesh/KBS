// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridMovementService.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/FlankCellDefinitions.h"
#include "GameplayTypes/TacticalMovementConstants.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"


UTacGridMovementService::UTacGridMovementService()
{
}

void UTacGridMovementService::Initialize(UGridDataManager* InDataManager)
{
	checkf(InDataManager, TEXT("UTacGridMovementService: DataManager must not be null"));
	DataManager = InDataManager;
}

//=============================================================================
// Validation Helpers
//=============================================================================

bool UTacGridMovementService::ValidateMovementParameters(AUnit* Unit, FTacCoordinates Where) const
{
	checkf(Unit, TEXT("UTacGridMovementService: Unit must not be null"));

	if (!Where.IsValidCell())
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Invalid target cell [%d,%d]"), Where.Row, Where.Col);
		return false;
	}

	return true;
}

bool UTacGridMovementService::GetUnitCurrentPosition(AUnit* Unit, FTacCoordinates& OutPos) const
{
	if (!Unit->GridMetadata.IsOnField())
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Unit is not on field"));
		return false;
	}
	OutPos = Unit->GridMetadata.Coords;
	return true;
}

bool UTacGridMovementService::ValidateAndGetPosition(AUnit* Unit, FTacCoordinates Where, FTacCoordinates& OutCurrentPos) const
{
	return ValidateMovementParameters(Unit, Where) && GetUnitCurrentPosition(Unit, OutCurrentPos);
}

//=============================================================================
// Movement Execution Helpers
//=============================================================================

bool UTacGridMovementService::ExecuteDataMove(AUnit* Unit, FTacCoordinates From, FTacCoordinates To)
{
	if (!DataManager->RemoveUnit(Unit))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to remove unit from [%d,%d]"), From.Row, From.Col);
		return false;
	}

	if (!DataManager->PlaceUnit(Unit, To))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to place unit at [%d,%d]"), To.Row, To.Col);
		DataManager->PlaceUnit(Unit, From);
		return false;
	}
	return true;
}

bool UTacGridMovementService::ExecuteSwapMove(AUnit* Unit1, FTacCoordinates Pos1, AUnit* Unit2, FTacCoordinates Pos2)
{
	// Remove both units first
	if (!DataManager->RemoveUnit(Unit1))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to remove unit from [%d,%d]"), Pos1.Row, Pos1.Col);
		return false;
	}

	if (!DataManager->RemoveUnit(Unit2))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to remove unit from [%d,%d]"), Pos2.Row, Pos2.Col);
		// Restore Unit1 to maintain consistency
		DataManager->PlaceUnit(Unit1, Pos1);
		return false;
	}

	// Place both units in swapped positions
	if (!DataManager->PlaceUnit(Unit1, Pos2))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to place unit at [%d,%d]"), Pos2.Row, Pos2.Col);
		// Restore both units
		DataManager->PlaceUnit(Unit1, Pos1);
		DataManager->PlaceUnit(Unit2, Pos2);
		return false;
	}

	if (!DataManager->PlaceUnit(Unit2, Pos1))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to place unit at [%d,%d]"), Pos1.Row, Pos1.Col);
		// Restore both units
		DataManager->RemoveUnit(Pos2);
		DataManager->PlaceUnit(Unit1, Pos1);
		DataManager->PlaceUnit(Unit2, Pos2);
		return false;
	}

	return true;
}

FTacMovementSegment UTacGridMovementService::CreateMovementSegment(FVector Start, FVector End, float Speed, FRotator TargetRotation)
{
	const float Distance = FVector::Dist(Start, End);
	const float Duration = (Speed > 0.0f) ? (Distance / Speed) : FTacMovementConstants::FallbackMovementDuration;

	return FTacMovementSegment(Start, End, Duration, TargetRotation);
}

//=============================================================================
// Visual Generation Helpers
//=============================================================================

TArray<FTacMovementSegment> UTacGridMovementService::BuildPathSegments(AUnit* Unit, FTacCoordinates Where)
{
	const FVector StartLocation = Unit->GetActorLocation();
	const FVector TargetLocation = DataManager->GetCellWorldLocation(Where);
	const FVector Direction = (TargetLocation - StartLocation).GetSafeNormal();
	const float Speed = Unit->GetMovementSpeed();

	FRotator TargetRotation = Direction.Rotation();
	TargetRotation.Yaw += FTacMovementConstants::ModelForwardOffset;

	return { CreateMovementSegment(StartLocation, TargetLocation, Speed, TargetRotation) };
}

FTacMovementVisualData UTacGridMovementService::BuildVisualData(const TArray<FTacMovementSegment>& Segments, FTacCoordinates FinalCell, ETeamSide TeamSide, bool bIsFlankCell, bool bIsMultiCell)
{
	FTacMovementVisualData VisualData;
	VisualData.Segments = Segments;
	VisualData.CurrentSegmentProgress = 0.0f;

	// Rotation responsibility split:
	// - Flank cells: enemy-dependent rotation (both 1-cell and 2-cell rotate when entering flank)
	// - Center cells, 1-cell: snap to team-based default orientation
	// - Center cells, 2-cell: no rotation (orientation is set at placement/flank-entry only)
	if (bIsFlankCell)
	{
		VisualData.bApplyFlankRotationAtEnd = true;
		VisualData.TargetRotation = FinalCell.GetFlankRotation();
	}
	else if (!bIsMultiCell)
	{
		VisualData.bApplyDefaultRotationAtEnd = true;
		VisualData.TargetRotation = CalculateCellOrientation(TeamSide);
	}

	return VisualData;
}

//=============================================================================
// Orientation Calculation
//=============================================================================

FRotator UTacGridMovementService::CalculateCellOrientation(ETeamSide TeamSide) const
{
	// Only calculates simple team-based rotation for center cells
	// Does NOT handle flank rotation (requires enemy adjacency info)
	const float Yaw = (TeamSide == ETeamSide::Attacker)
		? FTacMovementConstants::AttackerDefaultYaw
		: FTacMovementConstants::DefenderDefaultYaw;
	return FRotator(0.0f, Yaw, 0.0f);
}

//=============================================================================
// Public Methods
//=============================================================================

TArray<FTacMovementSegment> UTacGridMovementService::MakeUnitPath(AUnit* UnitToMove, FTacCoordinates Where)
{
	const FTacCoordinates EffectiveTarget = UnitToMove->GridMetadata.ResolveMovementTarget(Where);
	FTacCoordinates CurrentPos;
	if (!ValidateAndGetPosition(UnitToMove, EffectiveTarget, CurrentPos))
	{
		return {};
	}
	return BuildPathSegments(UnitToMove, EffectiveTarget);
}

FTacMovementVisualData UTacGridMovementService::MakeMovementVisual(AUnit* UnitToMove, FTacCoordinates Where)
{
	const FTacCoordinates EffectiveTarget = UnitToMove->GridMetadata.ResolveMovementTarget(Where);
	FTacCoordinates CurrentPos;
	if (!ValidateAndGetPosition(UnitToMove, EffectiveTarget, CurrentPos))
	{
		return {};
	}

	TArray<FTacMovementSegment> Path = BuildPathSegments(UnitToMove, EffectiveTarget);
	if (Path.Num() == 0)
	{
		return {};
	}

	return BuildVisualData(Path, EffectiveTarget, UnitToMove->GetTeamSide(), EffectiveTarget.IsFlankCell(), UnitToMove->GridMetadata.IsMultiCell());
}

bool UTacGridMovementService::PushUnitToCell(AUnit* UnitToMove, FTacCoordinates Where)
{
	const FTacCoordinates EffectiveTarget = UnitToMove->GridMetadata.ResolveMovementTarget(Where);
	FTacCoordinates CurrentPos;
	if (!ValidateAndGetPosition(UnitToMove, EffectiveTarget, CurrentPos))
	{
		return false;
	}

	// Low-level grid data manipulation - no game logic, VFX, or side effects
	if (!ExecuteDataMove(UnitToMove, CurrentPos, EffectiveTarget))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to push unit to cell"));
		return false;
	}

	UE_LOG(LogTacGrid, Log, TEXT("PushUnitToCell: %s [%d,%d]->[%d,%d]"), *UnitToMove->GetLogName(), CurrentPos.Row, CurrentPos.Col, EffectiveTarget.Row, EffectiveTarget.Col);
	return true;
}

bool UTacGridMovementService::TeleportUnit(AUnit* UnitToMove, FTacCoordinates Where)
{
	const FTacCoordinates EffectiveTarget = UnitToMove->GridMetadata.ResolveMovementTarget(Where);
	FTacCoordinates CurrentPos;
	if (!ValidateAndGetPosition(UnitToMove, EffectiveTarget, CurrentPos))
	{
		return false;
	}

	// Game mechanic teleport - future: add VFX, sound, ability triggers
	if (!ExecuteDataMove(UnitToMove, CurrentPos, EffectiveTarget))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to execute teleport"));
		return false;
	}
	UnitToMove->HandleMoved(FTacMovementVisualData{});

	UE_LOG(LogTacGrid, Log, TEXT("TeleportUnit: %s [%d,%d]->[%d,%d]"), *UnitToMove->GetLogName(), CurrentPos.Row, CurrentPos.Col, EffectiveTarget.Row, EffectiveTarget.Col);
	return true;
}

bool UTacGridMovementService::MoveUnit(AUnit* Unit, FTacCoordinates Where)
{
	// ===== VALIDATION PHASE =====
	const FTacCoordinates EffectiveTarget = Unit->GridMetadata.ResolveMovementTarget(Where);
	FTacCoordinates CurrentPos;
	if (!ValidateAndGetPosition(Unit, EffectiveTarget, CurrentPos))
	{
		return false;
	}

	AUnit* TargetOccupant = DataManager->GetUnit(EffectiveTarget);

	// Swaps are only valid between two 1-cell units
	const bool bIsSwap = (TargetOccupant != nullptr);
	if (bIsSwap && (Unit->GridMetadata.IsMultiCell() || TargetOccupant->GridMetadata.IsMultiCell()))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Swap not allowed when either unit is 2-cell"));
		return false;
	}

	// For 2-cell movers: ensure the secondary target cell is also free
	if (Unit->GridMetadata.IsMultiCell())
	{
		const FTacCoordinates NewSecondary = GetExtraCellCoords(EffectiveTarget, Unit->GridMetadata.Orientation);
		if (NewSecondary.IsValidCell())
		{
			AUnit* SecondaryOccupant = DataManager->GetUnit(NewSecondary);
			if (SecondaryOccupant != nullptr && SecondaryOccupant != Unit)
			{
				UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: 2-cell secondary target [%d,%d] blocked"), NewSecondary.Row, NewSecondary.Col);
				return false;
			}
		}
	}

	// ===== VISUAL GENERATION PHASE (no mutation) =====
	const bool bTargetIsFlank = EffectiveTarget.IsFlankCell();
	const bool bCurrentIsFlank = CurrentPos.IsFlankCell();

	FTacMovementVisualData UnitVisuals = BuildVisualData(BuildPathSegments(Unit, EffectiveTarget), EffectiveTarget, Unit->GetTeamSide(), bTargetIsFlank, Unit->GridMetadata.IsMultiCell());
	FTacMovementVisualData SwappedVisuals;
	if (bIsSwap)
	{
		SwappedVisuals = BuildVisualData(BuildPathSegments(TargetOccupant, CurrentPos), CurrentPos, TargetOccupant->GetTeamSide(), bCurrentIsFlank, false);
	}

	// ===== GRID MUTATION PHASE =====
	if (bIsSwap)
	{
		if (!ExecuteSwapMove(Unit, CurrentPos, TargetOccupant, EffectiveTarget))
		{
			UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to execute swap move"));
			return false;
		}
		Unit->HandleMoved(UnitVisuals);
		TargetOccupant->HandleMoved(SwappedVisuals);
	}
	else
	{
		if (!ExecuteDataMove(Unit, CurrentPos, EffectiveTarget))
		{
			UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to execute data move"));
			return false;
		}
		Unit->HandleMoved(UnitVisuals);
	}

	const FString SwapSuffix = bIsSwap ? FString::Printf(TEXT(" (swap with %s)"), *TargetOccupant->GetLogName()) : TEXT("");
	UE_LOG(LogTacGrid, Log, TEXT("MoveUnit: %s [%d,%d]->[%d,%d]%s"),
		*Unit->GetLogName(), CurrentPos.Row, CurrentPos.Col, EffectiveTarget.Row, EffectiveTarget.Col, *SwapSuffix);
	return true;
}
