#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "SummonSpellAbilityDefinition.generated.h"

class AUnit;
class UUnitDefinition;

UCLASS(BlueprintType)
class KBS_API USummonSpellAbilityDefinition : public UUnitAbilityDefinition
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Summon")
	TSubclassOf<AUnit> SummonedUnitClass;

	UPROPERTY(EditAnywhere, Category = "Summon")
	TObjectPtr<UUnitDefinition> SummonedUnitDefinition;

	// 0 = permanent until caster death or battle end
	UPROPERTY(EditAnywhere, Category = "Summon")
	int32 SummonDurationTurns = 0;

	UPROPERTY(EditAnywhere, Category = "Summon")
	bool bDespawnOnCasterDeath = true;

	UPROPERTY(EditAnywhere, Category = "Summon")
	bool bReplacePreviousSummon = true;
};
