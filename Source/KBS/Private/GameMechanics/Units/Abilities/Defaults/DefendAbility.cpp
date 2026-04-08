#include "GameMechanics/Units/Abilities/Defaults/DefendAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/Tags/Tactical/AbilityTags.h"


FAbilityExecutionResult UDefendAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	
	Owner->GetStats().Status.SetDefending();
	ConsumeCharge();
	SetCompletionTag();
	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UDefendAbility::CanExecute(FTacCoordinates TargetCell) const
{
	return Owner && RemainingCharges > 0 && OwnerCanAct() && CanActByContext();
}

bool UDefendAbility::CanExecute() const
{
	return Owner && RemainingCharges > 0 && OwnerCanAct() && CanActByContext();
}

FGameplayTagContainer UDefendAbility::BuildTags() const
{
	FGameplayTagContainer Tags = Super::BuildTags();
	Tags.AddTag(TAG_ABILITY_DEFEND);
	return Tags;
}

