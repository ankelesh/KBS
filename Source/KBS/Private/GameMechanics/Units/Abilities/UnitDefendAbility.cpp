#include "GameMechanics/Units/Abilities/UnitDefendAbility.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"


FAbilityResult UUnitDefendAbility::ApplyAbilityEffect(AUnit* SourceUnit, FTacCoordinates TargetCell)
{
	if (!SourceUnit)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom, FText::FromString("No source unit"));
	}

	SourceUnit->SetDefending(true);
	UE_LOG(LogTemp, Log, TEXT("%s is now defending - incoming damage will be halved"), *SourceUnit->GetName());

	FAbilityResult Result = CreateSuccessResult();
	Result.UnitsAffected.Add(SourceUnit);
	return Result;
}
