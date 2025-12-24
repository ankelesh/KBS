#pragma once
#include "CoreMinimal.h"
#include "BattleEffect.h"
#include "StatModBattleEffectDataAsset.h"
#include "StatModBattleEffect.generated.h"

UCLASS(Blueprintable)
class KBS_API UStatModBattleEffect : public UBattleEffect
{
	GENERATED_BODY()
public:
	virtual void Initialize(UBattleEffectDataAsset* InConfig, UWeapon* InWeapon, AUnit* InAttacker) override;
	virtual void OnTurnEnd(AUnit* Owner) override;
	virtual void OnApplied(AUnit* Owner) override;
	virtual void OnRemoved(AUnit* Owner) override;
	virtual bool HandleReapply(UBattleEffect* NewEffect, AUnit* Owner) override;
protected:
	UStatModBattleEffectDataAsset* GetStatModConfig() const { return Cast<UStatModBattleEffectDataAsset>(Config); }
	void ApplyStatModifications(AUnit* Owner);
	void RemoveStatModifications(AUnit* Owner);
};
