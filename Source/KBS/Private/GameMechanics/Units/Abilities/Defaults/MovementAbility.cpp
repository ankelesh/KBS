#include "GameMechanics/Units/Abilities/Defaults/MovementAbility.h"
#include "GameMechanics/Units/Abilities/Defaults/MovementAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/Tags/Tactical/AbilityTags.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridMovementService.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameplayTypes/GridCoordinates.h"

FTargetingDescriptor UMovementAbility::GetTargeting() const
{
	const UMovementAbilityDefinition* MoveDef = CastChecked<UMovementAbilityDefinition>(Config);
	checkf(Owner, TEXT("UMovementAbility::GetTargeting called without an owner"));
	return Owner->GetGridMetadata().Coords.Layer == ETacGridLayer::Air
		? MoveDef->AirTargeting
		: MoveDef->GroundTargeting;
}

FAbilityExecutionResult UMovementAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	UTacGridMovementService* MovementService = GetMovementService();
	check(MovementService);
	
	const UMovementAbilityDefinition* MoveDef = CastChecked<UMovementAbilityDefinition>(Config);
	const bool bSuccess = MoveDef->bIsAnimated
		? MovementService->MoveUnit(Owner, TargetCell)
		: MovementService->TeleportUnit(Owner, TargetCell);
	check(bSuccess);
	ConsumeCharge();
	SetCompletionTag();
	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool UMovementAbility::CanExecute(FTacCoordinates TargetCell) const
{
	check(Owner);
	if (RemainingCharges <= 0) return false;
	if (!CanActByContext()) return false;
	if (!Owner->GetStats().Status.CanMove()) return false;
	UTacGridTargetingService* TargetingService = GetTargetingService();
	check(TargetingService);
	return TargetingService->HasValidTargetAtCell(Owner, TargetCell, GetTargeting());
}

bool UMovementAbility::CanExecute() const
{
	check(Owner);
	if (!RemainingCharges) return false;
	if (!CanActByContext()) return false;
	if (!Owner->GetStats().Status.CanMove()) return false;
	UTacGridTargetingService* TargetingService = GetTargetingService();
	check(TargetingService);
	return TargetingService->HasAnyValidTargets(Owner, GetTargeting());
}

FGameplayTagContainer UMovementAbility::BuildTags() const
{
	FGameplayTagContainer Tags = Super::BuildTags();
	Tags.AddTag(TAG_ABILITY_MOVEMENT);
	return Tags;
}


