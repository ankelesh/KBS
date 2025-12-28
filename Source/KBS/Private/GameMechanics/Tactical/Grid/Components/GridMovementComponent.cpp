#include "GameMechanics/Tactical/Grid/Components/GridMovementComponent.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/FlankCellDefinitions.h"
#include "GameMechanics/Units/LargeUnit.h"
UGridMovementComponent::UGridMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}
void UGridMovementComponent::Initialize(ATacBattleGrid* InGrid, UGridDataManager* InDataManager)
{
	Grid = InGrid;
	DataManager = InDataManager;
}

bool UGridMovementComponent::MoveUnit(AUnit* Unit, int32 TargetRow, int32 TargetCol)
{
	if (!Unit || !Grid || !DataManager)
	{
		UE_LOG(LogTemp, Error, TEXT("MoveUnit: Invalid parameters!"));
		return false;
	}
	int32 CurrentRow, CurrentCol;
	EBattleLayer Layer;
	if (!DataManager->GetUnitPosition(Unit, CurrentRow, CurrentCol, Layer))
	{
		UE_LOG(LogTemp, Error, TEXT("MoveUnit: Could not find unit position!"));
		return false;
	}
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

	if (Unit->IsMultiCell() && TargetOccupant)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveUnit: Multi-cell units cannot swap positions!"));
		return false;
	}

	if (TargetOccupant)
	{
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
		DataManager->RemoveUnit(TargetRow, TargetCol, Layer, Grid);
		DataManager->RemoveUnit(CurrentRow, CurrentCol, Layer, Grid);
		DataManager->PlaceUnit(Unit, TargetRow, TargetCol, Layer, Grid);
		DataManager->PlaceUnit(TargetOccupant, CurrentRow, CurrentCol, Layer, Grid);
		if (bLeavingFlank && !bEnteringFlank)
		{
			RestoreOriginalRotation(Unit);
			OnUnitFlankStateChanged.Broadcast(Unit, false, FIntPoint(CurrentCol, CurrentRow));
		}
		else if (bEnteringFlank)
		{
			ApplyFlankRotation(Unit, TargetRow, TargetCol);
			OnUnitFlankStateChanged.Broadcast(Unit, true, FIntPoint(TargetCol, TargetRow));
		}
		if (bSwappedLeavingFlank && !bSwappedEnteringFlank)
		{
			RestoreOriginalRotation(TargetOccupant);
			OnUnitFlankStateChanged.Broadcast(TargetOccupant, false, FIntPoint(TargetCol, TargetRow));
		}
		else if (bSwappedEnteringFlank)
		{
			ApplyFlankRotation(TargetOccupant, CurrentRow, CurrentCol);
			OnUnitFlankStateChanged.Broadcast(TargetOccupant, true, FIntPoint(CurrentCol, CurrentRow));
		}
	}
	else
	{
		DataManager->RemoveUnit(CurrentRow, CurrentCol, Layer, Grid);
		DataManager->PlaceUnit(Unit, TargetRow, TargetCol, Layer, Grid);
		UE_LOG(LogTemp, Log, TEXT("MoveUnit: Done move"));
		if (bLeavingFlank && !bEnteringFlank)
		{
			RestoreOriginalRotation(Unit);
			OnUnitFlankStateChanged.Broadcast(Unit, false, FIntPoint(CurrentCol, CurrentRow));
		}
		else if (bEnteringFlank)
		{
			ApplyFlankRotation(Unit, TargetRow, TargetCol);
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
				if (Data.bNeedsFinalRotation)
				{
					Unit->VisualsComponent->RotateTowardTarget(Data.FinalRotation, 360.0f);
					Data.bNeedsFinalRotation = false;
				}
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
void UGridMovementComponent::ApplyFlankRotation(AUnit* Unit, int32 Row, int32 Col)
{
	if (!Unit || !DataManager)
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("ApplyFlankRotation: Unit entered flank cell at [%d,%d]"), Row, Col);
	if (!DataManager->IsUnitOnFlank(Unit))
	{
		DataManager->SetUnitOriginalRotation(Unit, Unit->GetActorRotation());
	}
	const FRotator FlankRotation = FGridCoordinates::GetFlankRotation(Row, Col);
	Unit->SetActorRotation(FlankRotation);
	DataManager->SetUnitFlankState(Unit, true);
}
void UGridMovementComponent::RestoreOriginalRotation(AUnit* Unit)
{
	if (!Unit || !DataManager || !DataManager->IsUnitOnFlank(Unit))
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("RestoreOriginalRotation: Unit exited flank cell"));
	Unit->SetActorRotation(DataManager->GetUnitOriginalRotation(Unit));
	DataManager->SetUnitFlankState(Unit, false);
}
