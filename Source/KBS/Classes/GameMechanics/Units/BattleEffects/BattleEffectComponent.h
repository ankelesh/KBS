#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTypes/TacMovementTypes.h"
#include "GameplayTags.h"
#include "GameplayTypes/EffectTypes.h"
#include "BattleEffectComponent.generated.h"
class UBattleEffect;
class AUnit;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KBS_API UBattleEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBattleEffectComponent();
	virtual void BeginPlay() override;
	bool AddEffect(UBattleEffect* Effect);
	void RemoveEffect(UBattleEffect* Effect);
	void RemoveEffect(FName StackingId);
	void RemoveEffect(const FGuid EffectId);
	void ClearAllEffects();
	const TArray<TObjectPtr<UBattleEffect>>& GetActiveEffects() const { return ActiveEffects; }

	// Quering
	bool HasEffectWithTag(const FGameplayTag& Tag) const;
	int32 CountEffectsWithTag(const FGameplayTag& Tag) const;
	TArray<UBattleEffect*> GetEffectsMatching(
		TOptional<FGameplayTag> Tag,
		TOptional<EEffectPolarity> PolarityFilter = {},
		TOptional<bool> DispellableFilter = {}) const;
	int32 DispelEffects(
		TOptional<EEffectPolarity> PolarityFilter = {},
		TOptional<FGameplayTag> TagFilter = {},
		int32 MaxToRemove = MAX_int32);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	TArray<TObjectPtr<UBattleEffect>> ActiveEffects;

private:
	AUnit* GetOwnerUnit() const;
	UFUNCTION()
	void OnOwnerTurnStart(AUnit* Unit);
	UFUNCTION()
	void OnOwnerTurnEnd(AUnit* Unit);
	UFUNCTION()
	void OnOwnerAttacked(AUnit* Victim, AUnit* Attacker);
	UFUNCTION()
	void OnOwnerAttacks(AUnit* Attacker, AUnit* Target);
	UFUNCTION()
	void OnOwnerMoved(AUnit* Unit, const FTacMovementVisualData& MovementData);
	UFUNCTION()
	void OnOwnerDied(AUnit* Unit);
	
	void BroadcastToEffects(TFunctionRef<void(UBattleEffect*)> Notify, bool bCheckExpiry = true);
	UBattleEffect* FindByStackingId(FName StackingId) const;
	int32 CountByStackingId(FName StackingId) const;
	void ApplyEffect(UBattleEffect* Effect);
	void ExecuteReapplyDecision(EReapplyDecision Decision,
		UBattleEffect* OldEffect, UBattleEffect* NewEffect);
};
