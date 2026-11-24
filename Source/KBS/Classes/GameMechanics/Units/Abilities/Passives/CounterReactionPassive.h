#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "CounterReactionPassive.generated.h"

/**
 * Answers every damaging hit on the owner with a reaction attack against the attacker, up to MaxUses
 * times per turn (0 = unlimited). Uses refill through the base per-turn restore.
 */
UCLASS()
class KBS_API UCounterReactionPassive : public UUnitAbility
{
	GENERATED_BODY()
public:
	virtual void InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner) override;
	virtual void Subscribe() override;
	virtual void Unsubscribe() override;
	virtual bool CanExecute() const override { return false; }

private:
	UFUNCTION()
	void HandleOwnerDamaged(AUnit* Unit, AUnit* Attacker);

	UPROPERTY()
	TObjectPtr<UCombatDescriptor> OwnDescriptor;
};
