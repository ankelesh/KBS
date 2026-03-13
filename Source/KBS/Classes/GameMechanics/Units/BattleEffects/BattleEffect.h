#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameplayTypes/EffectTypes.h"
#include "GameplayTypes/TargetingDescriptor.h"
#include "BattleEffectDataAsset.h"
#include "BattleEffect.generated.h"
class AUnit;

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
	virtual EReapplyDecision HandleReapply(UBattleEffect* NewEffect);
	
	// Triggers
	virtual void OnApplied() {}
	virtual void OnRemoved();
	virtual void OnTurnStart() {}
	virtual void OnTurnEnd() {}
	virtual void OnUnitAttacked(AUnit* AttackingUnit) {}
	virtual void OnUnitAttacks(AUnit* Target) {}
	virtual void OnUnitMoved() {}
	virtual void OnUnitDied() {}
	
	// Getters
	FTargetingDescriptor GetEffectTargeting() const;
	EDamageSource GetDamageSource() const;
	FText GetEffectName() const { return Config->Name; }
	int32 GetDuration() const { return Duration; }
	EEffectStackPolicy GetStackingPolicy() const { return Config->StackPolicy; }
	void RefreshDuration(int32 InDuration) { Duration = FMath::Max(InDuration, Duration); }
	bool IsExpired() const { return Duration <= 0; }
	FName GetStackingId() const { return Config ? Config->StackingId : NAME_None; }
	EEffectStackPolicy GetStackPolicy() const { return Config->StackPolicy; }
	int32 GetStackLimit() const { return Config ? Config->MaxStacks : 0; }
	UBattleEffectDataAsset* GetConfig() const { return Config; }
	bool IsRequringRoll() const { return Config->bIsAccuracyDependent; }
	bool IsDispellable() const { return Config->bIsDispellable; }
	EEffectPolarity GetPolarity() const { return Config->Polarity; }
	AUnit* GetOwner() const { return Owner; }
	const FGuid& GetEffectId() const { return EffectId; }
	bool HasTag(const FGameplayTag& Tag) const { return Config && Config->EffectTags.HasTag(Tag); }
	bool HasAnyTag(const FGameplayTagContainer& Tags) const { return Config && Config->EffectTags.HasAny(Tags); }
	
	
	UPROPERTY(BlueprintAssignable)
	FOnEffectDurationChange OnDurationChange;
	UPROPERTY(BlueprintAssignable)
	FOnEffectRemoved OnEffectRemoved;
protected:
	void DecrementDuration() { if (Duration > 0) Duration--; BroadcastDurationChange(); }
	void BroadcastDurationChange() const { OnDurationChange.Broadcast(Duration);}
	virtual void NotifyOnTriggered();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UBattleEffectDataAsset> Config;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	int32 Duration = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<AUnit> Owner;
	UPROPERTY(SaveGame)
	FGuid EffectId;
};
