#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Combat/CombatDescriptorData.h"
#include "GameMechanics/Units/Stats/UnitInstantChange.h"
#include "HistoryActionPassiveDefinition.generated.h"

/** Base for passives reacting to the owner's UUnitActionHistoryComponent. */
UCLASS(Abstract, BlueprintType)
class KBS_API UHistoryActionPassiveDefinition : public UUnitAbilityDefinition
{
	GENERATED_BODY()
public:
	UHistoryActionPassiveDefinition()
	{
		bIsPassive = true;
		Targeting = ETargetReach::Self;
	}

	// Must be one of the history component's tracked actions.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger")
	FGameplayTag TriggerTag;

	// When true the passive fires only if this action differs from the previous one.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger")
	bool bRequireChange = true;
};

UCLASS(BlueprintType)
class KBS_API UHistoryActionModifyStatsPassiveDefinition : public UHistoryActionPassiveDefinition
{
	GENERATED_BODY()
public:
	UHistoryActionModifyStatsPassiveDefinition();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action Effect")
	FUnitInstantChange Change;
};

UCLASS(BlueprintType)
class KBS_API UHistoryActionApplyEffectPassiveDefinition : public UHistoryActionPassiveDefinition
{
	GENERATED_BODY()
public:
	UHistoryActionApplyEffectPassiveDefinition();

	// Applied to the owner. Duration comes from each effect asset.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action Effect")
	TArray<FDescriptorEffectConfig> Effects;
};
