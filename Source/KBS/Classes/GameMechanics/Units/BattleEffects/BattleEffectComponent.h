#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BattleEffectComponent.generated.h"
class UBattleEffect;
class AUnit;
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KBS_API UBattleEffectComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UBattleEffectComponent();
	void AddEffect(UBattleEffect* Effect);
	void RemoveEffect(UBattleEffect* Effect);
	void ClearAllEffects();
	void BroadcastTurnStart();
	void BroadcastTurnEnd();
	void BroadcastAttacked(AUnit* Attacker);
	void BroadcastAttacks(AUnit* Target);
	void BroadcastMoved();
	void BroadcastDied();
	const TArray<TObjectPtr<UBattleEffect>>& GetActiveEffects() const { return ActiveEffects; }
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	TArray<TObjectPtr<UBattleEffect>> ActiveEffects;
private:
	AUnit* GetOwnerUnit() const;
};
