#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "SummonSpellAbility.generated.h"

class USummonSpellAbilityDefinition;

UCLASS(Blueprintable)
class KBS_API USummonSpellAbility : public UUnitAbility
{
	GENERATED_BODY()
public:
	virtual void InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner) override;

	virtual FAbilityExecutionResult Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override;
	virtual bool CanExecute() const override;
	virtual FTargetingDescriptor GetTargeting() const override;
	virtual FGameplayTagContainer BuildTags() const override;

private:
	UPROPERTY()
	TObjectPtr<USummonSpellAbilityDefinition> SummonConfig;

	UPROPERTY()
	TWeakObjectPtr<AUnit> ActiveSummon;

	void DespawnActiveSummon();
};
