#include "GameMechanics/Units/Abilities/Defaults/FleeAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/Tags/Tactical/AbilityTags.h"
#include "GameMechanics/Units/Components/UnitVisualsComponent.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameplayTypes/TacticalMovementConstants.h"


FAbilityExecutionResult UFleeAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	// 1) Turn unit to its field side — reverse of normal combat facing
	const float FleeYaw = (Owner->GetTeamSide() == ETeamSide::Attacker)
		? FTacMovementConstants::DefenderDefaultYaw
		: FTacMovementConstants::AttackerDefaultYaw;
	Owner->GetVisualsComponent()->RotateTowardTarget(FRotator(0.f, FleeYaw, 0.f));
	if (Owner->GetGridMetadata().HasExtraCell())
		Owner->GetVisualsComponent()->OnRotationCompletedNative.AddUObject(this, &UFleeAbility::OnFleeRotationCompleted);

	Owner->GetStats().Status.SetFleeing();
	Owner->OnUnitTurnStart.AddDynamic(this, &UFleeAbility::HandleTurnStarted);
	ConsumeCharge();
	SetCompletionTag();
	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UFleeAbility::CanExecute(FTacCoordinates TargetCell) const
{
	return CanExecute();
}

bool UFleeAbility::CanExecute() const
{
	check(Owner);
	return OwnerCanAct() && CanActByContext() && RemainingCharges > 0;
}

void UFleeAbility::OnFleeRotationCompleted()
{
	Owner->GetVisualsComponent()->OnRotationCompletedNative.RemoveAll(this);
	Owner->GetVisualsComponent()->ReverseExtraCellOffset();
}

void UFleeAbility::HandleTurnStarted(AUnit* Unit)
{
	// 4) Remove to off-field only if unit is still fleeing
	if (Owner->GetStats().Status.IsFleeing() && Owner->GetStats().Status.CanAct() && Owner->GetStats().Status.CanMove())
	{
		GetGridSubsystem()->PlaceUnitOffField(Owner);
	}

	// 5) One-shot — unsubscribe regardless of whether removal happened
	Owner->OnUnitTurnStart.RemoveDynamic(this, &UFleeAbility::HandleTurnStarted);
}

FGameplayTagContainer UFleeAbility::BuildTags() const
{
	FGameplayTagContainer Tags = Super::BuildTags();
	Tags.AddTag(TAG_ABILITY_FLEE);
	return Tags;
}
