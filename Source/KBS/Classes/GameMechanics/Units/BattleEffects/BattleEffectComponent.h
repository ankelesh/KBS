#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTypes/TacMovementTypes.h"
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
	void ClearAllEffects();
	const TArray<TObjectPtr<UBattleEffect>>& GetActiveEffects() const { return ActiveEffects; }
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	TArray<TObjectPtr<UBattleEffect>> ActiveEffects;
private:
	AUnit* GetOwnerUnit() const;
	UFUNCTION() void OnOwnerTurnStart(AUnit* Unit);
	UFUNCTION() void OnOwnerTurnEnd(AUnit* Unit);
	UFUNCTION() void OnOwnerAttacked(AUnit* Victim, AUnit* Attacker);
	UFUNCTION() void OnOwnerAttacks(AUnit* Attacker, AUnit* Target);
	UFUNCTION() void OnOwnerMoved(AUnit* Unit, const FTacMovementVisualData& MovementData);
	UFUNCTION() void OnOwnerDied(AUnit* Unit);
};
