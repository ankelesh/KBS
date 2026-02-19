// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridMovementService.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/FlankCellDefinitions.h"
#include "GameplayTypes/TacticalMovementConstants.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"

static FString UnitStr(const AUnit* Unit)
{
	if (!Unit) return TEXT("null");
	const UUnitDefinition* Def = Unit->GetUnitDefinition();
	const FString& Name = Def ? Def->UnitName : TEXT("?");
	return FString::Printf(TEXT("%s [%s]"), *Name, *Unit->GetUnitID().ToString().Left(8));
}


UTacGridMovementService::UTacGridMovementService()
{
}

void UTacGridMovementService::Initialize(UGridDataManager* InDataManager)
{
	DataManager = InDataManager;
}

//=============================================================================
// Validation Helpers
//=============================================================================

bool UTacGridMovementService::ValidateMovementParameters(AUnit* Unit, FTacCoordinates Where) const
{
	if (!Unit || !DataManager)
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Invalid Unit or DataManager"));
		return false;
	}

	if (!DataManager->IsValidCell(Where))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Invalid target cell [%d,%d]"), Where.Row, Where.Col);
		return false;
	}

	return true;
}

bool UTacGridMovementService::GetUnitCurrentPosition(AUnit* Unit, FTacCoordinates& OutPos) const
{
	ETacGridLayer Layer;
	if (!DataManager->GetUnitPosition(Unit, OutPos, Layer))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Could not find unit position"));
		return false;
	}
	OutPos.Layer = Layer;
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
	if (!DataManager)
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: DataManager null in ExecuteDataMove"));
		return false;
	}

	if (!DataManager->RemoveUnit(From))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to remove unit from [%d,%d]"), From.Row, From.Col);
		return false;
	}

	if (!DataManager->PlaceUnit(Unit, To))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to place unit at [%d,%d]"), To.Row, To.Col);
		return false;
	}

	return true;
}

bool UTacGridMovementService::ExecuteSwapMove(AUnit* Unit1, FTacCoordinates Pos1, AUnit* Unit2, FTacCoordinates Pos2)
{
	if (!DataManager)
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: DataManager null in ExecuteSwapMove"));
		return false;
	}

	// Remove both units first
	if (!DataManager->RemoveUnit(Pos1))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to remove unit from [%d,%d]"), Pos1.Row, Pos1.Col);
		return false;
	}

	if (!DataManager->RemoveUnit(Pos2))
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

FTacMovementVisualData UTacGridMovementService::BuildVisualData(const TArray<FTacMovementSegment>& Segments, FTacCoordinates FinalCell, ETeamSide TeamSide, bool bIsFlankCell)
{
	FTacMovementVisualData VisualData;
	VisualData.Segments = Segments;
	VisualData.CurrentSegmentProgress = 0.0f;

	// Convert FTacCoordinates (Row, Col) to FIntPoint (X=Col, Y=Row) for rendering
	VisualData.FinalCell = FIntPoint(FinalCell.Col, FinalCell.Row);

	VisualData.UnitTeamSide = TeamSide;
	VisualData.RotationDuration = FTacMovementConstants::DefaultRotationDuration;

	// Rotation responsibility split:
	// - Center cells: Simple team-based rotation calculated here
	// - Flank cells: Complex enemy-dependent rotation handled by caller
	if (bIsFlankCell)
	{
		VisualData.bApplyFlankRotationAtEnd = true;
		VisualData.bApplyDefaultRotationAtEnd = false;
		// TargetRotation not set - caller must compute based on adjacent enemies
	}
	else
	{
		VisualData.bApplyFlankRotationAtEnd = false;
		VisualData.bApplyDefaultRotationAtEnd = true;
		// TargetRotation set directly - simple team-based facing
		VisualData.TargetRotation = CalculateCellOrientation(FinalCell, TeamSide);
	}

	VisualData.RotationProgress = 0.0f;

	return VisualData;
}

//=============================================================================
// Orientation Calculation
//=============================================================================

FRotator UTacGridMovementService::CalculateCellOrientation(FTacCoordinates Location, ETeamSide TeamSide) const
{
	// Only calculates simple team-based rotation for center cells
	// Does NOT handle flank rotation (requires enemy adjacency info)
	FRotator DefaultRotation;
	DefaultRotation.Pitch = 0.0f;
	DefaultRotation.Roll = 0.0f;
	DefaultRotation.Yaw = (TeamSide == ETeamSide::Attacker)
		? FTacMovementConstants::AttackerDefaultYaw
		: FTacMovementConstants::DefenderDefaultYaw;

	return DefaultRotation;
}

//=============================================================================
// Public Methods
//=============================================================================

TArray<FTacMovementSegment> UTacGridMovementService::MakeUnitPath(AUnit* UnitToMove, FTacCoordinates Where)
{
	TArray<FTacMovementSegment> Path;

	FTacCoordinates CurrentPos;
	if (!ValidateAndGetPosition(UnitToMove, Where, CurrentPos))
	{
		return Path;
	}

	const FVector StartLocation = UnitToMove->GetActorLocation();
	const FVector TargetLocation = DataManager->GetCellWorldLocation(Where);
	const FVector Direction = (TargetLocation - StartLocation).GetSafeNormal();
	const float Speed = UnitToMove->GetMovementSpeed();

	FRotator TargetRotation = Direction.Rotation();
	TargetRotation.Yaw += FTacMovementConstants::ModelForwardOffset;

	Path.Add(CreateMovementSegment(StartLocation, TargetLocation, Speed, TargetRotation));

	return Path;
}

FTacMovementVisualData UTacGridMovementService::MakeMovementVisual(AUnit* UnitToMove, FTacCoordinates Where)
{
	FTacMovementVisualData VisualData;

	FTacCoordinates CurrentPos;
	if (!ValidateAndGetPosition(UnitToMove, Where, CurrentPos))
	{
		return VisualData;
	}

	TArray<FTacMovementSegment> Path = MakeUnitPath(UnitToMove, Where);
	if (Path.Num() == 0)
	{
		return VisualData;
	}

	const bool bIsFlankCell = DataManager && DataManager->IsFlankCell(Where);
	return BuildVisualData(Path, Where, UnitToMove->GetTeamSide(), bIsFlankCell);
}

bool UTacGridMovementService::PushUnitToCell(AUnit* UnitToMove, FTacCoordinates Where)
{
	FTacCoordinates CurrentPos;
	if (!ValidateAndGetPosition(UnitToMove, Where, CurrentPos))
	{
		return false;
	}

	// Low-level grid data manipulation - no game logic, VFX, or side effects
	if (!ExecuteDataMove(UnitToMove, CurrentPos, Where))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to push unit to cell"));
		return false;
	}

	UE_LOG(LogTacGrid, Log, TEXT("PushUnitToCell: %s [%d,%d]->[%d,%d]"), *UnitStr(UnitToMove), CurrentPos.Row, CurrentPos.Col, Where.Row, Where.Col);
	return true;
}

bool UTacGridMovementService::TeleportUnit(AUnit* UnitToMove, FTacCoordinates Where)
{
	FTacCoordinates CurrentPos;
	if (!ValidateAndGetPosition(UnitToMove, Where, CurrentPos))
	{
		return false;
	}

	// Game mechanic teleport - future: add VFX, sound, ability triggers
	if (!ExecuteDataMove(UnitToMove, CurrentPos, Where))
	{
		UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to execute teleport"));
		return false;
	}

	UE_LOG(LogTacGrid, Log, TEXT("TeleportUnit: %s [%d,%d]->[%d,%d]"), *UnitStr(UnitToMove), CurrentPos.Row, CurrentPos.Col, Where.Row, Where.Col);
	return true;
}

bool UTacGridMovementService::MoveUnit(AUnit* Unit, FTacCoordinates Where, FTacMovementVisualData& OutVisuals, TOptional<FTacMovementVisualData>& OutSwappedUnitVisuals)
{
	// ===== VALIDATION PHASE =====
	FTacCoordinates CurrentPos;
	if (!ValidateAndGetPosition(Unit, Where, CurrentPos))
	{
		return false;
	}

	AUnit* TargetOccupant = DataManager->GetUnit(Where);

	// Check multi-cell restrictions
	if (Unit->IsMultiCell() && TargetOccupant)
	{
		UE_LOG(LogTacGrid, Warning, TEXT("UTacGridMovementService: Multi-cell units cannot swap positions"));
		return false;
	}

	if (TargetOccupant && TargetOccupant->IsMultiCell())
	{
		UE_LOG(LogTacGrid, Warning, TEXT("UTacGridMovementService: Cannot swap with multi-cell unit"));
		return false;
	}

	// ===== VISUAL GENERATION PHASE (no mutation) =====
	const bool bIsSwap = (TargetOccupant != nullptr);

	const bool bTargetIsFlank = DataManager && DataManager->IsFlankCell(Where);
	const bool bCurrentIsFlank = DataManager && DataManager->IsFlankCell(CurrentPos);

	TArray<FTacMovementSegment> MainUnitPath = MakeUnitPath(Unit, Where);
	OutVisuals = BuildVisualData(MainUnitPath, Where, Unit->GetTeamSide(), bTargetIsFlank);

	if (bIsSwap)
	{
		TArray<FTacMovementSegment> SwappedUnitPath = MakeUnitPath(TargetOccupant, CurrentPos);
		OutSwappedUnitVisuals = BuildVisualData(SwappedUnitPath, CurrentPos, TargetOccupant->GetTeamSide(), bCurrentIsFlank);
	}
	else
	{
		OutSwappedUnitVisuals.Reset();
	}

	// ===== GRID MUTATION PHASE =====
	if (bIsSwap)
	{
		if (!ExecuteSwapMove(Unit, CurrentPos, TargetOccupant, Where))
		{
			UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to execute swap move"));
			return false;
		}
	}
	else
	{
		if (!ExecuteDataMove(Unit, CurrentPos, Where))
		{
			UE_LOG(LogTacGrid, Error, TEXT("UTacGridMovementService: Failed to execute data move"));
			return false;
		}
	}

	if (bIsSwap)
	{
		UE_LOG(LogTacGrid, Log, TEXT("MoveUnit: %s [%d,%d]->[%d,%d] (swap with %s)"),
			*UnitStr(Unit), CurrentPos.Row, CurrentPos.Col, Where.Row, Where.Col, *UnitStr(TargetOccupant));
	}
	else
	{
		UE_LOG(LogTacGrid, Log, TEXT("MoveUnit: %s [%d,%d]->[%d,%d]"),
			*UnitStr(Unit), CurrentPos.Row, CurrentPos.Col, Where.Row, Where.Col);
	}
	return true;
}
