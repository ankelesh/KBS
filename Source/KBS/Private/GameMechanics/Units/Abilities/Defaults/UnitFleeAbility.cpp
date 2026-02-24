#include "GameMechanics/Units/Abilities/Defaults/UnitFleeAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameplayTypes/TacticalMovementConstants.h"


bool UUnitFleeAbility::Execute(FTacCoordinates TargetCell)
{
	if (!Owner) return false;

	// 1) Turn unit to its field side — reverse of normal combat facing
	const float FleeYaw = (Owner->GetTeamSide() == ETeamSide::Attacker)
		? FTacMovementConstants::DefenderDefaultYaw
		: FTacMovementConstants::AttackerDefaultYaw;
	Owner->GetVisualsComponent()->RotateTowardTarget(FRotator(0.f, FleeYaw, 0.f));

	// 2) Mark as fleeing
	Owner->GetStats().Status.SetFleeing();

	// 3) Queue off-field removal for when this unit's next turn starts
	Owner->OnUnitTurnStart.AddDynamic(this, &UUnitFleeAbility::HandleTurnStarted);

	// Invalidate ability before lending turn back
	ConsumeCharge();

	// 6) End turn
	Owner->GetStats().Status.SetFocus();

	return true;
}

bool UUnitFleeAbility::CanExecute(FTacCoordinates TargetCell) const
{
	return CanExecute();
}

bool UUnitFleeAbility::CanExecute() const
{
	return Owner && RemainingCharges > 0;
}

void UUnitFleeAbility::HandleTurnStarted(AUnit* Unit)
{
	// 4) Remove to off-field only if unit is still fleeing
	if (Owner->GetStats().Status.IsFleeing())
	{
		GetGridSubsystem()->PlaceUnitOffField(Owner);
	}

	// 5) One-shot — unsubscribe regardless of whether removal happened
	Owner->OnUnitTurnStart.RemoveDynamic(this, &UUnitFleeAbility::HandleTurnStarted);
}
