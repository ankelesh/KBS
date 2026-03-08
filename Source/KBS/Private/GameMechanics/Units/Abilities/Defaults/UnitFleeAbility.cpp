#include "GameMechanics/Units/Abilities/Defaults/UnitFleeAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameplayTypes/TacticalMovementConstants.h"


FAbilityExecutionResult UUnitFleeAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	// 1) Turn unit to its field side — reverse of normal combat facing
	const float FleeYaw = (Owner->GetTeamSide() == ETeamSide::Attacker)
		? FTacMovementConstants::DefenderDefaultYaw
		: FTacMovementConstants::AttackerDefaultYaw;
	Owner->GetVisualsComponent()->RotateTowardTarget(FRotator(0.f, FleeYaw, 0.f));
	if (Owner->GetGridMetadata().HasExtraCell())
		Owner->GetVisualsComponent()->OnRotationCompletedNative.AddUObject(this, &UUnitFleeAbility::OnFleeRotationCompleted);

	Owner->GetStats().Status.SetFleeing();
	Owner->OnUnitTurnStart.AddDynamic(this, &UUnitFleeAbility::HandleTurnStarted);
	ConsumeCharge();
	SetCompletionTag();
	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UUnitFleeAbility::CanExecute(FTacCoordinates TargetCell) const
{
	return CanExecute();
}

bool UUnitFleeAbility::CanExecute() const
{
	check(Owner);
	return OwnerCanAct() && CanActByContext() && RemainingCharges > 0;
}

void UUnitFleeAbility::OnFleeRotationCompleted()
{
	Owner->GetVisualsComponent()->OnRotationCompletedNative.RemoveAll(this);
	Owner->GetVisualsComponent()->ReverseExtraCellOffset();
}

void UUnitFleeAbility::HandleTurnStarted(AUnit* Unit)
{
	// 4) Remove to off-field only if unit is still fleeing
	if (Owner->GetStats().Status.IsFleeing() && Owner->GetStats().Status.CanAct() && Owner->GetStats().Status.CanMove())
	{
		GetGridSubsystem()->PlaceUnitOffField(Owner);
	}

	// 5) One-shot — unsubscribe regardless of whether removal happened
	Owner->OnUnitTurnStart.RemoveDynamic(this, &UUnitFleeAbility::HandleTurnStarted);
}
