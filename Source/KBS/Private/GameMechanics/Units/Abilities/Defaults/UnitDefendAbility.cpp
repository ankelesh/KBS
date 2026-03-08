#include "GameMechanics/Units/Abilities/Defaults/UnitDefendAbility.h"
#include "GameMechanics/Units/Unit.h"


FAbilityExecutionResult UUnitDefendAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	
	Owner->GetStats().Status.SetDefending();
	ConsumeCharge();
	SetCompletionTag();
	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UUnitDefendAbility::CanExecute(FTacCoordinates TargetCell) const
{
	return Owner && RemainingCharges > 0 && OwnerCanAct() && CanActByContext();
}

bool UUnitDefendAbility::CanExecute() const
{
	return Owner && RemainingCharges > 0 && OwnerCanAct() && CanActByContext();
}

