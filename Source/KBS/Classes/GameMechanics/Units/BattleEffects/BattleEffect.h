#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTypes/DamageTypes.h"
#include "BattleEffectDataAsset.h"
#include "BattleEffect.generated.h"
class AUnit;
class UNiagaraComponent;
class UWeapon;

typedef TArray<TObjectPtr<UBattleEffect>> BattleEffectArray;
UCLASS(Abstract, Blueprintable)
class KBS_API UBattleEffect : public UObject
{
	GENERATED_BODY()
public:
	virtual void Initialize(UBattleEffectDataAsset* InConfig, UWeapon* InWeapon, AUnit* InAttacker);
	virtual void OnTurnStart(AUnit* Owner) {}
	virtual void OnTurnEnd(AUnit* Owner) {}
	virtual void OnUnitAttacked(AUnit* Owner, AUnit* AttackingUnit) {}
	virtual void OnUnitAttacks(AUnit* Owner, AUnit* Target) {}
	virtual void OnUnitMoved(AUnit* Owner) {}
	virtual void OnUnitDied(AUnit* Owner) {}
	virtual void OnApplied(AUnit* Owner) {}
	virtual void OnRemoved(AUnit* Owner) {}
	EDamageSource GetDamageSource() const;
	EEffectTarget GetEffectTarget() const;
	FText GetEffectName() const { return Config->Name; }
	int32 GetRemainingTurns() const { return RemainingTurns; }
	bool IsExpired() const { return RemainingTurns <= 0; }
	FName GetStackingId() const { return Config ? Config->StackingId : NAME_None; }
	virtual bool HandleReapply(UBattleEffect* NewEffect, AUnit* Owner);
	UBattleEffectDataAsset* GetConfig() const { return Config; }
	UWeapon* GetSourceWeapon() const { return SourceWeapon; }
	AUnit* GetAttacker() const { return Attacker; }
protected:
	void DecrementDuration() { if (RemainingTurns > 0) RemainingTurns--; }
	void SpawnEffectVFX(AUnit* Owner);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UBattleEffectDataAsset> Config;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	int32 RemainingTurns = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UWeapon> SourceWeapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<AUnit> Attacker;
};
