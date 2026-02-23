#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "SummonSpellAbility.generated.h"

class USummonSpellAbilityDefinition;

UCLASS(Blueprintable)
class KBS_API USummonSpellAbility : public UUnitAbilityInstance
{
	GENERATED_BODY()
public:
	virtual void InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner) override;

	virtual bool Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override;
	virtual bool CanExecute() const override;
	virtual ETargetReach GetTargeting() const override;

private:
	UPROPERTY()
	TObjectPtr<USummonSpellAbilityDefinition> SummonConfig;

	UPROPERTY()
	TWeakObjectPtr<AUnit> ActiveSummon;

	void DespawnActiveSummon();
};
