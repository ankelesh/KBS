#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "SummonedPassiveAbility.generated.h"

UCLASS()
class KBS_API USummonedPassiveAbility : public UUnitAbilityInstance
{
	GENERATED_BODY()
public:
	// Called instead of InitializeFromDefinition — no definition asset needed.
	void InitAsSummonedPassive(AUnit* InOwner, AUnit* InSummoner,
	                           bool bInDespawnOnSummonerDeath, int32 InDurationTurns);

	virtual void Subscribe() override;
	virtual void Unsubscribe() override;
	virtual bool IsPassive() const override { return true; }
	virtual bool CanExecute() const override { return false; }
	// Called by AbilityInventory at turn end. Does NOT call Super (no charge restore).
	virtual void HandleTurnEnd() override;

private:
	UPROPERTY()
	TWeakObjectPtr<AUnit> Summoner;

	bool bDespawnOnSummonerDeath = true;
	int32 RemainingTurns = 0; // 0 = permanent

	UFUNCTION()
	void HandleSummonerDied(AUnit* Unit);
	UFUNCTION()
	void HandleOwnerDied(AUnit* Unit);
	void DespawnSelf();
};
