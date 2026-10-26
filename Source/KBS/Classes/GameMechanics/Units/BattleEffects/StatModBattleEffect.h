#pragma once
#include "CoreMinimal.h"
#include "BattleEffect.h"
#include "StatModBattleEffectDataAsset.h"
#include "GameMechanics/Units/Stats/UnitStatDelta.h"
#include "StatModBattleEffect.generated.h"

UCLASS(Blueprintable)
class KBS_API UStatModBattleEffect : public UBattleEffect
{
	GENERATED_BODY()
public:
	virtual void Initialize(UBattleEffectDataAsset* InConfig) override;
	virtual void OnTurnEnd() override;
	virtual void OnApplied() override;
	virtual void OnRemoved() override;
	virtual EReapplyDecision HandleReapply(UBattleEffect* NewEffect) override;
protected:
	UStatModBattleEffectDataAsset* GetStatModConfig() const { return Cast<UStatModBattleEffectDataAsset>(Config); }
	void ApplyStatModifications();
	void RemoveStatModifications();

	UPROPERTY()
	FUnitStatDelta AppliedDelta;
};
