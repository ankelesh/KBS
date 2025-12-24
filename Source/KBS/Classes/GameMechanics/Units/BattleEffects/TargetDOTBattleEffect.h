#pragma once
#include "CoreMinimal.h"
#include "BattleEffect.h"
#include "DOTBattleEffectDataAsset.h"
#include "TargetDOTBattleEffect.generated.h"
UCLASS(Blueprintable)
class KBS_API UTargetDOTBattleEffect : public UBattleEffect
{
	GENERATED_BODY()
public:
	virtual void Initialize(UBattleEffectDataAsset* InConfig, UWeapon* InWeapon, AUnit* InAttacker) override;
	virtual void OnTurnEnd(AUnit* Owner) override;
	virtual void OnApplied(AUnit* Owner) override;
	virtual void OnRemoved(AUnit* Owner) override;
	virtual bool HandleReapply(UBattleEffect* NewEffect, AUnit* Owner) override;
protected:
	UDOTBattleEffectDataAsset* GetDOTConfig() const { return Cast<UDOTBattleEffectDataAsset>(Config); }
};
