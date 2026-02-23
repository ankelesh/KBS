#include "GameMechanics/Units/Abilities/Defaults/UnitMovementAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridMovementService.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"


bool UUnitMovementAbility::Execute(FTacCoordinates TargetCell)
{
	if (!Owner || RemainingCharges <= 0 || IsOutsideFocus()) return false;

	UTacGridMovementService* MovementService = GetMovementService();
	if (!MovementService) return false;

	bool bMoveSuccess = MovementService->MoveUnit(Owner, TargetCell);

	if (bMoveSuccess)
	{
		Owner->GetStats().Status.SetFocus();
		ConsumeCharge();
	}

	return bMoveSuccess;
}

bool UUnitMovementAbility::CanExecute(FTacCoordinates TargetCell) const
{
	if (!Owner || RemainingCharges <= 0 || IsOutsideFocus()) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return false;

	TArray<FTacCoordinates> ValidCells = TargetingService->GetValidTargetCells(Owner, GetTargeting());
	return ValidCells.Contains(TargetCell);
}

bool UUnitMovementAbility::CanExecute() const
{
	if (!Owner || RemainingCharges <= 0 || IsOutsideFocus()) return false;

	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return false;

	TArray<FTacCoordinates> ValidCells = TargetingService->GetValidTargetCells(Owner, GetTargeting());
	return ValidCells.Num() > 0;
}


