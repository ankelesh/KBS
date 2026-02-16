#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTypes/DamageTypes.h"
#include "BattleEffectDataAsset.h"
#include "BattleEffect.generated.h"
class AUnit;
class UNiagaraComponent;

typedef TArray<TObjectPtr<UBattleEffect>> BattleEffectArray;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectDurationChange, int32, NewDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectRemoved, UBattleEffect*, Effect);

UCLASS(Abstract, Blueprintable)
class KBS_API UBattleEffect : public UObject
{
	GENERATED_BODY()
public:
	virtual void Initialize(UBattleEffectDataAsset* InConfig);
	void SetOwner(AUnit* InOwner) { Owner = InOwner; }
	// Lifetime
	virtual void PrepareForApply(AUnit* Source, AUnit* Target) {}
	virtual bool HandleReapply(UBattleEffect* NewEffect);
	
	// Triggers
	virtual void OnApplied() {}
	virtual void OnRemoved();
	virtual void OnTurnStart() {}
	virtual void OnTurnEnd() {}
	virtual void OnUnitAttacked(AUnit* AttackingUnit) {}
	virtual void OnUnitAttacks(AUnit* Target) {}
	virtual void OnUnitMoved() {}
	virtual void OnUnitDied() {}
	EDamageSource GetDamageSource() const;
	
	// Getters
	ETargetReach GetEffectTargetReach() const;
	FText GetEffectName() const { return Config->Name; }
	int32 GetDuration() const { return Duration; }
	bool IsExpired() const { return Duration <= 0; }
	FName GetStackingId() const { return Config ? Config->StackingId : NAME_None; }
	UBattleEffectDataAsset* GetConfig() const { return Config; }
	bool IsRequringRoll() const { return bIsAccuracyDependent; }
	AUnit* GetOwner() const { return Owner; }
	const FGuid& GetEffectId() const { return EffectId; }
	
	UPROPERTY(BlueprintAssignable)
	FOnEffectDurationChange OnDurationChange;
	UPROPERTY(BlueprintAssignable)
	FOnEffectRemoved OnEffectRemoved;
protected:
	void DecrementDuration() { if (Duration > 0) Duration--; BroadcastDurationChange(); }
	void BroadcastDurationChange() const { OnDurationChange.Broadcast(Duration);}
	virtual void SpawnEffectVFX();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UBattleEffectDataAsset> Config;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	int32 Duration = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<AUnit> Owner;
	UPROPERTY(SaveGame)
	FGuid EffectId;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	EDamageSource DamageSource = EDamageSource::Physical;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	ETargetReach EffectTarget = ETargetReach::AnyEnemy;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	FName StackingId = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	int32 DefaultDuration = 3;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	/// No way to remove / never expires
	bool isImmutable = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	/// Can be removed by abilities
	bool bIsDispellable = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	bool bIsAccuracyDependent = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	EEffectType EffectType = EEffectType::Neutral;
};
