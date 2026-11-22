#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "ApplyEffectPassive.generated.h"

class UApplyEffectPassiveDefinition;

UCLASS()
class KBS_API UApplyEffectPassive : public UUnitAbility
{
	GENERATED_BODY()
public:
	virtual void Subscribe() override;
	virtual void Unsubscribe() override;
	virtual bool CanExecute() const override { return false; }

private:
	UFUNCTION()
	void ApplyEffects();

	const UApplyEffectPassiveDefinition& GetEffectsConfig() const;

	bool bEffectsApplied = false;
};
