#include "GameMechanics/Units/Abilities/UnitDefendAbility.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
FAbilityResult UUnitDefendAbility::ApplyAbilityEffect(const FAbilityBattleContext& Context)
{
	if (!Context.SourceUnit)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom, FText::FromString("No source unit"));
	}
	Context.SourceUnit->SetDefending(true);
	UE_LOG(LogTemp, Log, TEXT("%s is now defending - incoming damage will be halved"), *Context.SourceUnit->GetName());
	FAbilityResult Result = CreateSuccessResult();
	Result.UnitsAffected.Add(Context.SourceUnit);
	return Result;
}
