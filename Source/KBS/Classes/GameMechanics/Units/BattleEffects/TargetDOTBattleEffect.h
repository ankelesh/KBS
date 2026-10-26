#pragma once
#include "CoreMinimal.h"
#include "BattleEffect.h"
#include "DOTBattleEffectDataAsset.h"
#include "TargetDOTBattleEffect.generated.h"

class UCombatDescriptor;

UCLASS(Blueprintable)
class KBS_API UTargetDOTBattleEffect : public UBattleEffect
{
	GENERATED_BODY()
public:
	virtual void Initialize(UBattleEffectDataAsset* InConfig) override;
	virtual void PrepareForApply(AUnit* Source, AUnit* Target) override;
	virtual void OnTurnEnd() override;
	virtual void OnApplied() override;
	virtual void OnRemoved() override;
	virtual EReapplyDecision HandleReapply(UBattleEffect* NewEffect) override;
protected:
	UDOTBattleEffectDataAsset* GetDOTConfig() const { return Cast<UDOTBattleEffectDataAsset>(Config); }

	UPROPERTY()
	TObjectPtr<UCombatDescriptor> TickDescriptor;

	float StoredHitChance = 100.0f;
};
