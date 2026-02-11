#include "GameMechanics/Units/Abilities/UnitMovementAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridMovementService.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"


bool UUnitMovementAbility::Execute(FTacCoordinates TargetCell)
{
	if (!Owner) return false;

	UTacGridMovementService* MovementService = GetMovementService();
	if (!MovementService) return false;

	FTacMovementVisualData OutVisuals;
	TOptional<FTacMovementVisualData> OutSwappedVisuals;
	bool bMoveSuccess = MovementService->MoveUnit(Owner, TargetCell, OutVisuals, OutSwappedVisuals);

	if (bMoveSuccess)
	{
		Owner->GetStats().Status.SetFocus();
		ConsumeCharge();
	}

	return bMoveSuccess;
}

bool UUnitMovementAbility::CanExecute(FTacCoordinates TargetCell) const
{
	if (!Owner || RemainingCharges <= 0) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return false;

	TArray<FTacCoordinates> ValidCells = TargetingService->GetValidTargetCells(Owner, GetTargeting());
	return ValidCells.Contains(TargetCell);
}

bool UUnitMovementAbility::CanExecute() const
{
	if (!Owner || RemainingCharges <= 0) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return false;

	TArray<FTacCoordinates> ValidCells = TargetingService->GetValidTargetCells(Owner, GetTargeting());
	return ValidCells.Num() > 0;
}

void UUnitMovementAbility::Subscribe()
{
	if (Owner)
	{
		Owner->OnUnitTurnEnd.AddDynamic(this, &UUnitMovementAbility::HandleTurnEnd);
	}
}

void UUnitMovementAbility::Unsubscribe()
{
	if (Owner)
	{
		Owner->OnUnitTurnEnd.RemoveDynamic(this, &UUnitMovementAbility::HandleTurnEnd);
	}
}

void UUnitMovementAbility::HandleTurnEnd(AUnit*)
{
	RestoreCharges();
}

