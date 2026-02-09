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
	virtual void Initialize(UBattleEffectDataAsset* InConfig) override;
	virtual void OnTurnEnd() override;
	virtual void OnApplied() override;
	virtual void OnRemoved() override;
	virtual bool HandleReapply(UBattleEffect* NewEffect) override;
protected:
	UDOTBattleEffectDataAsset* GetDOTConfig() const { return Cast<UDOTBattleEffectDataAsset>(Config); }
};
