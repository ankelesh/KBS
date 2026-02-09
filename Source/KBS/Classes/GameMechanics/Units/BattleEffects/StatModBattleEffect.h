#pragma once
#include "CoreMinimal.h"
#include "BattleEffect.h"
#include "StatModBattleEffectDataAsset.h"
#include "GameplayTypes/DamageTypes.h"
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
	virtual bool HandleReapply(UBattleEffect* NewEffect) override;
protected:
	UStatModBattleEffectDataAsset* GetStatModConfig() const { return Cast<UStatModBattleEffectDataAsset>(Config); }
	void ApplyStatModifications();
	void RemoveStatModifications();

	// Cache applied modifications for exact removal
	UPROPERTY()
	int32 AppliedMaxHealthMod = 0;
	UPROPERTY()
	int32 AppliedInitiativeMod = 0;
	UPROPERTY()
	int32 AppliedAccuracyMod = 0;
	UPROPERTY()
	TArray<EDamageSource> AppliedImmunities;
	UPROPERTY()
	TMap<EDamageSource, int32> AppliedArmourMods;
};
