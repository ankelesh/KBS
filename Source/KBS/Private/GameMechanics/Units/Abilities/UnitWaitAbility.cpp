#include "GameMechanics/Units/Abilities/UnitWaitAbility.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
UUnitWaitAbility::UUnitWaitAbility()
{
	TurnAction = EAbilityTurnAction::Wait;
}
FAbilityResult UUnitWaitAbility::ApplyAbilityEffect(const FAbilityBattleContext& Context)
{
	if (!Context.SourceUnit)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom, FText::FromString("No source unit"));
	}
	UE_LOG(LogTemp, Log, TEXT("%s uses Wait - will be reinserted into queue with negated initiative"),
		*Context.SourceUnit->GetName());
	FAbilityResult Result = CreateSuccessResult();
	Result.UnitsAffected.Add(Context.SourceUnit);
	return Result;
}
