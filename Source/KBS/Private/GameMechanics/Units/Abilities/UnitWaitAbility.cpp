#include "GameMechanics/Units/Abilities/UnitWaitAbility.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"


UUnitWaitAbility::UUnitWaitAbility()
{
	TurnAction = EAbilityTurnAction::Wait;
}

FAbilityResult UUnitWaitAbility::ApplyAbilityEffect(AUnit* SourceUnit, FTacCoordinates TargetCell)
{
	if (!SourceUnit)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom, FText::FromString("No source unit"));
	}

	UE_LOG(LogTemp, Log, TEXT("%s uses Wait - will be reinserted into queue with negated initiative"),
		*SourceUnit->GetName());

	FAbilityResult Result = CreateSuccessResult();
	Result.UnitsAffected.Add(SourceUnit);
	return Result;
}

