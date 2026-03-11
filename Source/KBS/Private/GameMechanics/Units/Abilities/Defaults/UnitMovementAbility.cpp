#include "GameMechanics/Units/Abilities/Defaults/UnitMovementAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridMovementService.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"


FAbilityExecutionResult UUnitMovementAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	UTacGridMovementService* MovementService = GetMovementService();
	check(MovementService);
	
	bool bMoveSuccess = MovementService->MoveUnit(Owner, TargetCell);
	check(bMoveSuccess);
	ConsumeCharge();
	SetCompletionTag();
	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UUnitMovementAbility::CanExecute(FTacCoordinates TargetCell) const
{
	check(Owner);
	if (RemainingCharges <= 0) return false;
	if (!CanActByContext()) return false;
	if (!Owner->GetStats().Status.CanMove()) return false;
	UTacGridTargetingService* TargetingService = GetTargetingService();
	check(TargetingService);
	return TargetingService->HasValidTargetAtCell(Owner, TargetCell, GetTargeting());
}

bool UUnitMovementAbility::CanExecute() const
{
	check(Owner);
	if (!RemainingCharges) return false;
	if (!CanActByContext()) return false;
	if (!Owner->GetStats().Status.CanMove()) return false;
	UTacGridTargetingService* TargetingService = GetTargetingService();
	check(TargetingService);
	return TargetingService->HasAnyValidTargets(Owner, GetTargeting());
}


