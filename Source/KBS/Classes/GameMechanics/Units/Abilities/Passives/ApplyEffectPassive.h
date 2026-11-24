#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/Passives/BattleStartPassive.h"
#include "ApplyEffectPassive.generated.h"

class UApplyEffectPassiveDefinition;

UCLASS()
class KBS_API UApplyEffectPassive : public UBattleStartPassive
{
	GENERATED_BODY()
protected:
	virtual void OnBattleStart() override;

private:
	const UApplyEffectPassiveDefinition& GetEffectsConfig() const;
};
