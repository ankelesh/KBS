#include "GameMechanics/Tactical/Grid/Components/GridMovementComponent.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/FlankCellDefinitions.h"

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
	if (!DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
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
	if (!DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}

	UBattleTeam* UnitTeam = DataManager->GetTeamForUnit(Unit);

	const TArray<FIntPoint> AdjacentOffsets = {
		FIntPoint(0, -1), FIntPoint(0, 1), FIntPoint(-1, 0), FIntPoint(1, 0)
	};

	for (const FIntPoint& Offset : AdjacentOffsets)
	{
		const int32 TargetRow = UnitRow + Offset.Y;
		const int32 TargetCol = UnitCol + Offset.X;

		if (!DataManager->IsValidCell(TargetRow, TargetCol, UnitLayer))
		{
			continue;
		}

		// Block movement into own team's flank columns
		if (DataManager->IsFlankCell(TargetRow, TargetCol))
		{
			UBattleTeam* AttackerTeam = DataManager->GetAttackerTeam();

			// Attackers cannot enter their own flanks (columns 0-1)
			if (UnitTeam == AttackerTeam && FFlankCellDefinitions::IsAttackerFlankColumn(TargetCol))
			{
				continue;
			}

			// Defenders cannot enter their own flanks (columns 3-4)
			if (UnitTeam != AttackerTeam && FFlankCellDefinitions::IsDefenderFlankColumn(TargetCol))
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
	if (!DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}

	UBattleTeam* UnitTeam = DataManager->GetTeamForUnit(Unit);
	if (!UnitTeam)
	{
		return;
	}

	const FIntPoint UnitPos(UnitCol, UnitRow);
	UBattleTeam* AttackerTeam = DataManager->GetAttackerTeam();
	const bool bIsAttacker = (UnitTeam == AttackerTeam);

	// Determine enemy flank columns based on unit team
	TArray<int32> EnemyFlankColumns;
	if (bIsAttacker)
	{
		EnemyFlankColumns = FFlankCellDefinitions::DefenderFlankColumns;
	}
	else
	{
		EnemyFlankColumns = FFlankCellDefinitions::AttackerFlankColumns;
	}

	// Check all enemy flank cells on the grid
	for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
	{
		for (int32 Col : EnemyFlankColumns)
		{
			// Skip restricted cells (Z cells at row 2, columns 0 and 4)
			if (!DataManager->IsValidCell(Row, Col, UnitLayer))
			{
				continue;
			}

			// Skip if not a flank cell
			if (!DataManager->IsFlankCell(Row, Col))
			{
				continue;
			}

			const FIntPoint FlankCell(Col, Row);

			// Check if unit can enter this flank cell based on entry rules
			if (CanEnterFlankCell(UnitPos, FlankCell, UnitTeam))
			{
				// Check occupancy - allow if empty or friendly unit
				AUnit* Occupant = DataManager->GetUnit(Row, Col, UnitLayer);
				if (!Occupant || UnitTeam->ContainsUnit(Occupant))
				{
					OutCells.Add(FlankCell);
				}
			}
		}
	}
}

void UGridMovementComponent::GetAirMoveCells(AUnit* Unit, TArray<FIntPoint>& OutCells) const
{
	int32 UnitRow, UnitCol;
	EBattleLayer UnitLayer;
	if (!DataManager->GetUnitPosition(Unit, UnitRow, UnitCol, UnitLayer))
	{
		return;
	}

	UBattleTeam* UnitTeam = DataManager->GetTeamForUnit(Unit);

	for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
	{
		for (int32 Col = 0; Col < FGridCoordinates::GridSize; ++Col)
		{
			if (DataManager->IsRestrictedCell(Row, Col))
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
	if (!DataManager->GetUnitPosition(Unit, CurrentRow, CurrentCol, Layer))
	{
		UE_LOG(LogTemp, Error, TEXT("MoveUnit: Could not find unit position!"));
		return false;
	}

	// Calculate movement animation parameters BEFORE grid updates
	const FVector StartLocation = Unit->GetActorLocation();
	const FVector TargetLocation = DataManager->GetCellWorldLocation(TargetRow, TargetCol, Layer);
	const FVector Direction = (TargetLocation - StartLocation).GetSafeNormal();
	const float Distance = FVector::Dist(StartLocation, TargetLocation);
	const float MovementSpeed = Unit ? Unit->GetMovementSpeed() : 300.0f;

	FRotator TargetRotation = Direction.Rotation();
	TargetRotation.Yaw += ModelForwardOffset;
	if (Unit->VisualsComponent)
	{
		Unit->VisualsComponent->RotateTowardTarget(TargetRotation, 360.0f);
	}

	StartMovementInterpolation(Unit, StartLocation, TargetLocation, Direction, Distance, MovementSpeed, TargetRow, TargetCol);

	const bool bLeavingFlank = DataManager->IsFlankCell(CurrentRow, CurrentCol);
	const bool bEnteringFlank = DataManager->IsFlankCell(TargetRow, TargetCol);

	AUnit* TargetOccupant = DataManager->GetUnit(TargetRow, TargetCol, Layer);

	if (TargetOccupant)
	{
		// Handle animation for swapped unit
		const FVector SwapStartLocation = TargetOccupant->GetActorLocation();
		const FVector SwapTargetLocation = DataManager->GetCellWorldLocation(CurrentRow, CurrentCol, Layer);
		const FVector SwapDirection = (SwapTargetLocation - SwapStartLocation).GetSafeNormal();
		const float SwapDistance = FVector::Dist(SwapStartLocation, SwapTargetLocation);
		const float SwapMovementSpeed = TargetOccupant->GetMovementSpeed() ? TargetOccupant->GetMovementSpeed() : 300.0f;

		FRotator SwapRotation = SwapDirection.Rotation();
		SwapRotation.Yaw += ModelForwardOffset;
		if (TargetOccupant->VisualsComponent)
		{
			TargetOccupant->VisualsComponent->RotateTowardTarget(SwapRotation, 360.0f);
		}

		StartMovementInterpolation(TargetOccupant, SwapStartLocation, SwapTargetLocation, SwapDirection, SwapDistance, SwapMovementSpeed, CurrentRow, CurrentCol);

		const bool bSwappedLeavingFlank = DataManager->IsFlankCell(TargetRow, TargetCol);
		const bool bSwappedEnteringFlank = DataManager->IsFlankCell(CurrentRow, CurrentCol);

		// Swap units
		DataManager->RemoveUnit(TargetRow, TargetCol, Layer, Grid);
		DataManager->RemoveUnit(CurrentRow, CurrentCol, Layer, Grid);

		DataManager->PlaceUnit(Unit, TargetRow, TargetCol, Layer, Grid);
		DataManager->PlaceUnit(TargetOccupant, CurrentRow, CurrentCol, Layer, Grid);

		// Handle flank exit/entry for moving unit
		if (bLeavingFlank && !bEnteringFlank)
		{
			OnUnitFlankStateChanged.Broadcast(Unit, false, FIntPoint(CurrentCol, CurrentRow));
		}
		else if (bEnteringFlank)
		{
			OnUnitFlankStateChanged.Broadcast(Unit, true, FIntPoint(TargetCol, TargetRow));
		}

		// Handle flank exit/entry for swapped unit
		if (bSwappedLeavingFlank && !bSwappedEnteringFlank)
		{
			OnUnitFlankStateChanged.Broadcast(TargetOccupant, false, FIntPoint(TargetCol, TargetRow));
		}
		else if (bSwappedEnteringFlank)
		{
			OnUnitFlankStateChanged.Broadcast(TargetOccupant, true, FIntPoint(CurrentCol, CurrentRow));
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
			OnUnitFlankStateChanged.Broadcast(Unit, false, FIntPoint(CurrentCol, CurrentRow));
		}
		else if (bEnteringFlank)
		{
			OnUnitFlankStateChanged.Broadcast(Unit, true, FIntPoint(TargetCol, TargetRow));
		}
	}


	return true;
}

void UGridMovementComponent::StartMovementInterpolation(AUnit* Unit, FVector StartLocation, FVector TargetLocation, FVector Direction, float Distance, float Speed, int32 TargetRow, int32 TargetCol)
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
	InterpData.bNeedsFinalRotation = true;
	InterpData.UnitTeamSide = Unit->GetTeamSide();
	InterpData.FinalRotation = CalculateDefaultCellOrientation(TargetRow, TargetCol, Unit->GetTeamSide());

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

				// Start final rotation to default orientation
				if (Data.bNeedsFinalRotation)
				{
					Unit->VisualsComponent->RotateTowardTarget(Data.FinalRotation, 360.0f);
					Data.bNeedsFinalRotation = false;
				}

				// Only remove from movement tracking when rotation is also complete
				if (!Unit->VisualsComponent->IsRotating())
				{
					It.RemoveCurrent();
					continue;
				}
			}
			else
			{
				It.RemoveCurrent();
				continue;
			}
		}
	}
}

bool UGridMovementComponent::CanEnterFlankCell(const FIntPoint& UnitPos, const FIntPoint& FlankCell, UBattleTeam* UnitTeam) const
{
	if (!DataManager || !UnitTeam)
	{
		return false;
	}

	const int32 UnitRow = UnitPos.Y;
	const int32 UnitCol = UnitPos.X;
	const int32 FlankRow = FlankCell.Y;
	const int32 FlankCol = FlankCell.X;

	UBattleTeam* AttackerTeam = DataManager->GetAttackerTeam();
	const bool bIsAttacker = (UnitTeam == AttackerTeam);

	// Determine if this is an enemy flank
	const bool bIsEnemyFlank = (bIsAttacker && FFlankCellDefinitions::IsDefenderFlankColumn(FlankCol)) ||
	                           (!bIsAttacker && FFlankCellDefinitions::IsAttackerFlankColumn(FlankCol));

	if (!bIsEnemyFlank)
	{
		return false;
	}

	// Check if target is closest-to-center or far flank
	const bool bIsClosestFlank = FFlankCellDefinitions::IsClosestFlankColumn(FlankCol);
	const bool bIsFarFlank = FFlankCellDefinitions::IsFarFlankColumn(FlankCol);

	// Rule 1: Can enter closest-to-center enemy flank FROM center row (row 2, cols 1-3)
	if (bIsClosestFlank && FFlankCellDefinitions::IsCenterLineCell(UnitRow, UnitCol))
	{
		return IsAdjacentCell(UnitPos, FlankCell);
	}

	// Rule 2: Can enter closest-to-center enemy flank FROM adjacent friendly flank cell
	if (bIsClosestFlank && IsFlankCell(UnitPos))
	{
		const bool bIsOwnFlank = (bIsAttacker && FFlankCellDefinitions::IsAttackerFlankColumn(UnitCol)) ||
		                         (!bIsAttacker && FFlankCellDefinitions::IsDefenderFlankColumn(UnitCol));

		if (bIsOwnFlank && IsAdjacentCell(UnitPos, FlankCell))
		{
			return true;
		}
	}

	// Rule 3: Can enter far enemy flank FROM closest enemy flank
	if (bIsFarFlank && IsFlankCell(UnitPos))
	{
		const bool bIsInClosestEnemyFlank = (bIsAttacker && UnitCol == 3) || (!bIsAttacker && UnitCol == 1);

		if (bIsInClosestEnemyFlank && IsAdjacentCell(UnitPos, FlankCell))
		{
			return true;
		}
	}

	// Rule 4: Can enter far enemy flank FROM adjacent normal (non-flank) cell
	if (bIsFarFlank && !IsFlankCell(UnitPos))
	{
		return IsAdjacentCell(UnitPos, FlankCell);
	}

	return false;
}

bool UGridMovementComponent::IsAdjacentCell(const FIntPoint& CellA, const FIntPoint& CellB) const
{
	const int32 ManhattanDistance = FMath::Abs(CellA.X - CellB.X) + FMath::Abs(CellA.Y - CellB.Y);
	return ManhattanDistance == 1;
}

bool UGridMovementComponent::IsFlankCell(const FIntPoint& Cell) const
{
	if (!DataManager)
	{
		return false;
	}

	return DataManager->IsFlankCell(Cell.Y, Cell.X);
}

FRotator UGridMovementComponent::CalculateDefaultCellOrientation(int32 Row, int32 Col, ETeamSide TeamSide) const
{
	if (DataManager && DataManager->IsFlankCell(Row, Col))
	{
		const FIntPoint ClosestCell = FindClosestNonFlankCell(Row, Col);
		const FVector CurrentPos = DataManager->GetCellWorldLocation(Row, Col, EBattleLayer::Ground);
		const FVector TargetPos = DataManager->GetCellWorldLocation(ClosestCell.Y, ClosestCell.X, EBattleLayer::Ground);
		const FVector Direction = (TargetPos - CurrentPos).GetSafeNormal();

		FRotator Rotation = Direction.Rotation();
		Rotation.Yaw += ModelForwardOffset;
		return Rotation;
	}

	FRotator DefaultRotation;
	DefaultRotation.Pitch = 0.0f;
	DefaultRotation.Roll = 0.0f;
	DefaultRotation.Yaw = (TeamSide == ETeamSide::Attacker) ? AttackerDefaultYaw : DefenderDefaultYaw;

	return DefaultRotation;
}

FIntPoint UGridMovementComponent::FindClosestNonFlankCell(int32 FlankRow, int32 FlankCol) const
{
	int32 ClosestCol = 2;
	float MinDistance = FLT_MAX;

	const TArray<int32>& CenterCols = FFlankCellDefinitions::CenterColumns;
	for (int32 Col : CenterCols)
	{
		for (int32 Row = 0; Row < FGridCoordinates::GridSize; ++Row)
		{
			if (DataManager && DataManager->IsValidCell(Row, Col, EBattleLayer::Ground))
			{
				const float Distance = FMath::Sqrt(float(FMath::Square(Row - FlankRow) + FMath::Square(Col - FlankCol)));
				if (Distance < MinDistance)
				{
					MinDistance = Distance;
					ClosestCol = Col;
				}
			}
		}
	}

	return FIntPoint(ClosestCol, FlankRow);
}
