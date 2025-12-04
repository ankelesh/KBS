#include "GameMechanics/Tactical/Grid/Components/GridMovementComponent.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameplayTypes/GridCoordinates.h"

UGridMovementComponent::UGridMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGridMovementComponent::Initialize(ATacBattleGrid* InGrid, UGridDataManager* InDataManager)
{
	Grid = InGrid;
	DataManager = InDataManager;
}

TArray<FIntPoint> UGridMovementComponent::GetValidMoveCells(AUnit* Unit) const
{
	TArray<FIntPoint> ValidCells;

	if (!Unit || !Grid || !DataManager)
	{
		return ValidCells;
	}

	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!Grid->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return ValidCells;
	}

	if (UnitLayer == EBattleLayer::Air)
	{
		GetAirMoveCells(Unit, ValidCells);
	}
	else
	{
		GetAdjacentMoveCells(Unit, ValidCells);
		GetFlankMoveCells(Unit, ValidCells);
	}

	return ValidCells;
}

void UGridMovementComponent::GetAdjacentMoveCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!Grid->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}

	UBattleTeam* UnitTeam = Grid->GetTeamForUnit(Unit);

	const TArray<FIntPoint> AdjacentOffsets = {
		FIntPoint(0, -1), FIntPoint(0, 1), FIntPoint(-1, 0), FIntPoint(1, 0)
	};

	for (const FIntPoint& Offset : AdjacentOffsets)
	{
		const int32 TargetRow = UnitRow + Offset.Y;
		const int32 TargetCol = UnitCol + Offset.X;

		if (!Grid->IsValidCell(TargetRow, TargetCol, UnitLayer))
		{
			continue;
		}

		if (Grid->IsFlankCell(TargetRow, TargetCol))
		{
			UBattleTeam* AttackerTeam = Grid->GetTeamForUnit(Unit);
			UBattleTeam* DefenderTeam = Grid->GetEnemyTeam(Unit);

			if (UnitTeam == AttackerTeam && TargetRow <= 1)
			{
				continue;
			}

			if (UnitTeam == DefenderTeam && TargetRow >= 3)
			{
				continue;
			}
		}

		AUnit* OccupyingUnit = DataManager->GetUnit(TargetRow, TargetCol, UnitLayer);
		if (!OccupyingUnit || (UnitTeam && UnitTeam->ContainsUnit(OccupyingUnit)))
		{
			OutCells.Add(FIntPoint(TargetCol, TargetRow));
		}
	}
}

void UGridMovementComponent::GetFlankMoveCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!Grid->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}

	UBattleTeam* UnitTeam = Grid->GetTeamForUnit(Unit);
	UBattleTeam* EnemyTeam = Grid->GetEnemyTeam(Unit);

	if (!UnitTeam || !EnemyTeam)
	{
		return;
	}

	// Special flank movement: move to closest enemy flank cell only (not from center column)
	if (UnitCol == 2)
	{
		return; // Units in center column cannot move to flanks
	}

	// Determine which team and corresponding flank row
	int32 FlankRow = -1;
	TArray<FIntPoint> FlankCells;

	// Get the actual attacker and defender teams from grid
	UBattleTeam* AttackerTeam = (UnitTeam && EnemyTeam) ?
		(UnitTeam->GetTeamSide() == ETeamSide::Attacker ? UnitTeam : EnemyTeam) : nullptr;
	UBattleTeam* DefenderTeam = (UnitTeam && EnemyTeam) ?
		(UnitTeam->GetTeamSide() == ETeamSide::Defender ? UnitTeam : EnemyTeam) : nullptr;

	if (UnitTeam == AttackerTeam && AttackerTeam)
	{
		// Attackers can move to defender's flank at row 3, cols 0/4
		FlankRow = 3;
		FlankCells = { FIntPoint(0, FlankRow), FIntPoint(4, FlankRow) };
	}
	else if (UnitTeam == DefenderTeam && DefenderTeam)
	{
		// Defenders can move to attacker's flank at row 1, cols 0/4
		FlankRow = 1;
		FlankCells = { FIntPoint(0, FlankRow), FIntPoint(4, FlankRow) };
	}

	if (FlankRow == -1)
	{
		return;
	}

	// Find closest flank cell
	int32 MinDist = TNumericLimits<int32>::Max();
	FIntPoint ClosestFlank(-1, -1);

	for (const FIntPoint& FlankCell : FlankCells)
	{
		if (!Grid->IsValidCell(FlankCell.Y, FlankCell.X, UnitLayer))
		{
			continue;
		}

		const int32 Dist = FMath::Abs(FlankCell.X - UnitCol) + FMath::Abs(FlankCell.Y - UnitRow);
		if (Dist < MinDist)
		{
			MinDist = Dist;
			ClosestFlank = FlankCell;
		}
	}

	if (ClosestFlank.X >= 0)
	{
		AUnit* Occupant = DataManager->GetUnit(ClosestFlank.Y, ClosestFlank.X, UnitLayer);
		if (!Occupant || UnitTeam->ContainsUnit(Occupant))
		{
			OutCells.Add(ClosestFlank);
			UE_LOG(LogTemp, Log, TEXT("Unit at [%d,%d] can move to closest flank [%d,%d], distance: %d"),
				UnitRow, UnitCol, ClosestFlank.Y, ClosestFlank.X, MinDist);
		}
	}
}

void UGridMovementComponent::GetAirMoveCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!Grid->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}

	UBattleTeam* UnitTeam = Grid->GetTeamForUnit(Unit);

	for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridCoordinates::GridSize; ++Col)
		{
			if (Grid->IsRestrictedCell(Row, Col))
			{
				continue;
			}

			AUnit* OccupyingUnit = DataManager->GetUnit(Row, Col, UnitLayer);
			if (!OccupyingUnit || (UnitTeam && UnitTeam->ContainsUnit(OccupyingUnit)))
			{
				OutCells.Add(FIntPoint(Col, Row));
			}
		}
	}
}

bool UGridMovementComponent::MoveUnit(AUnit* Unit, int32 TargetRow, int32 TargetCol)
{
	if (!Unit || !Grid || !DataManager)
	{
		UE_LOG(LogTemp, Error, TEXT("MoveUnit: Invalid parameters!"));
		return false;
	}

	const TArray<FIntPoint> ValidCells = GetValidMoveCells(Unit);
	const FIntPoint TargetCell(TargetCol, TargetRow);

	UE_LOG(LogTemp, Log, TEXT("MoveUnit: Checking if [%d,%d] is valid. Valid cells count: %d"), TargetRow, TargetCol, ValidCells.Num());
	for (const FIntPoint& Cell : ValidCells)
	{
		UE_LOG(LogTemp, Log, TEXT("  Valid: [%d,%d]"), Cell.Y, Cell.X);
	}

	if (!ValidCells.Contains(TargetCell))
	{
		UE_LOG(LogTemp, Error, TEXT("MoveUnit: Target cell [%d,%d] is NOT in valid cells list!"), TargetRow, TargetCol);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("MoveUnit: Target is valid, executing move..."));

	int32 CurrentRow, CurrentCol;
	EBattleLayer Layer;
	if (!Grid->GetUnitPosition(Unit, CurrentRow, CurrentCol, Layer))
	{
		UE_LOG(LogTemp, Error, TEXT("MoveUnit: Could not find unit position!"));
		return false;
	}

	// Calculate movement animation parameters BEFORE grid updates
	const FVector StartLocation = Unit->GetActorLocation();
	const FVector TargetLocation = Grid->GetCellWorldLocation(TargetRow, TargetCol, Layer);
	const FVector Direction = (TargetLocation - StartLocation).GetSafeNormal();
	const float Distance = FVector::Dist(StartLocation, TargetLocation);
	const float MovementSpeed = Unit ? Unit->GetDisplayData().MoveSpeed: 300.0f;

	const FRotator TargetRotation = Direction.Rotation();
	if (Unit->VisualsComponent)
	{
		Unit->VisualsComponent->RotateTowardTarget(TargetRotation, 720.0f);
	}

	StartMovementInterpolation(Unit, StartLocation, TargetLocation, Direction, Distance, MovementSpeed);

	const bool bLeavingFlank = Grid->IsFlankCell(CurrentRow, CurrentCol);
	const bool bEnteringFlank = Grid->IsFlankCell(TargetRow, TargetCol);

	AUnit* TargetOccupant = DataManager->GetUnit(TargetRow, TargetCol, Layer);

	if (TargetOccupant)
	{
		// Handle animation for swapped unit
		const FVector SwapStartLocation = TargetOccupant->GetActorLocation();
		const FVector SwapTargetLocation = Grid->GetCellWorldLocation(CurrentRow, CurrentCol, Layer);
		const FVector SwapDirection = (SwapTargetLocation - SwapStartLocation).GetSafeNormal();
		const float SwapDistance = FVector::Dist(SwapStartLocation, SwapTargetLocation);
		const float SwapMovementSpeed = TargetOccupant->UnitDefinition ? TargetOccupant->UnitDefinition->MovementSpeed : 300.0f;

		const FRotator SwapRotation = SwapDirection.Rotation();
		if (TargetOccupant->VisualsComponent)
		{
			TargetOccupant->VisualsComponent->RotateTowardTarget(SwapRotation, 720.0f);
		}

		StartMovementInterpolation(TargetOccupant, SwapStartLocation, SwapTargetLocation, SwapDirection, SwapDistance, SwapMovementSpeed);

		const bool bSwappedLeavingFlank = Grid->IsFlankCell(TargetRow, TargetCol);
		const bool bSwappedEnteringFlank = Grid->IsFlankCell(CurrentRow, CurrentCol);

		// Swap units
		DataManager->RemoveUnit(TargetRow, TargetCol, Layer, Grid);
		DataManager->RemoveUnit(CurrentRow, CurrentCol, Layer, Grid);

		DataManager->PlaceUnit(Unit, TargetRow, TargetCol, Layer, Grid);
		DataManager->PlaceUnit(TargetOccupant, CurrentRow, CurrentCol, Layer, Grid);

		// Handle flank exit/entry for moving unit
		if (bLeavingFlank && !bEnteringFlank)
		{
			Grid->UnitExitsFlank(Unit);
		}
		else if (bEnteringFlank)
		{
			Grid->UnitEntersFlank(Unit, TargetRow, TargetCol);
		}

		// Handle flank exit/entry for swapped unit
		if (bSwappedLeavingFlank && !bSwappedEnteringFlank)
		{
			Grid->UnitExitsFlank(TargetOccupant);
		}
		else if (bSwappedEnteringFlank)
		{
			Grid->UnitEntersFlank(TargetOccupant, CurrentRow, CurrentCol);
		}
	}
	else
	{
		// Simple move
		DataManager->RemoveUnit(CurrentRow, CurrentCol, Layer, Grid);
		DataManager->PlaceUnit(Unit, TargetRow, TargetCol, Layer, Grid);
		UE_LOG(LogTemp, Log, TEXT("MoveUnit: Done move"));

		// Handle flank exit/entry
		if (bLeavingFlank && !bEnteringFlank)
		{
			Grid->UnitExitsFlank(Unit);
		}
		else if (bEnteringFlank)
		{
			Grid->UnitEntersFlank(Unit, TargetRow, TargetCol);
		}
	}

	// Broadcast movement completion for turn system
	if (ATacBattleGrid* BattleGrid = Cast<ATacBattleGrid>(Grid))
	{
		BattleGrid->OnMovementComplete.Broadcast();
	}

	return true;
}

void UGridMovementComponent::StartMovementInterpolation(AUnit* Unit, FVector StartLocation, FVector TargetLocation, FVector Direction, float Distance, float Speed)
{
	if (!Unit || Distance <= 0.0f || Speed <= 0.0f)
	{
		return;
	}

	FMovementInterpData InterpData;
	InterpData.StartLocation = StartLocation;
	InterpData.TargetLocation = TargetLocation;
	InterpData.Direction = Direction;
	InterpData.ElapsedTime = 0.0f;
	InterpData.Duration = Distance / Speed;

	UnitsBeingMoved.Add(Unit, InterpData);

	if (Unit->VisualsComponent)
	{
		Unit->VisualsComponent->SetIsMoving(true);
		Unit->VisualsComponent->SetMovementSpeed(Speed);
	}
}

void UGridMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (auto It = UnitsBeingMoved.CreateIterator(); It; ++It)
	{
		AUnit* Unit = It.Key();
		FMovementInterpData& Data = It.Value();

		if (!Unit)
		{
			It.RemoveCurrent();
			continue;
		}

		Data.ElapsedTime += DeltaTime;
		const float Alpha = FMath::Clamp(Data.ElapsedTime / Data.Duration, 0.0f, 1.0f);
		const float SmoothedAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

		const FVector NewLocation = FMath::Lerp(Data.StartLocation, Data.TargetLocation, SmoothedAlpha);
		Unit->SetActorLocation(NewLocation);

		if (Alpha >= 1.0f)
		{
			Unit->SetActorLocation(Data.TargetLocation);

			if (Unit->VisualsComponent)
			{
				Unit->VisualsComponent->SetIsMoving(false);
			}

			It.RemoveCurrent();
		}
	}
}
