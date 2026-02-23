#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "EvasiveStancePassive.generated.h"

/**
 * Passive: while active, reduces the accuracy of any attacker targeting the owner
 * by 20 (flat) during the calculation phase.
 *
 * Lifecycle per hit:
 *   Subscribe()             — binds to Owner->OnBeingTargetedInCalculation (non-dynamic)
 *   OnBeingTargeted()       — applies -20 flat modifier; registers with Hit.Interfere()
 *   HitTriggerCleanup(Hit)  — removes the modifier (called by ~FHitInstance)
 *   Unsubscribe()           — unbinds on ability removal
 */
UCLASS()
class KBS_API UEvasiveStancePassive : public UUnitAbilityInstance
{
	GENERATED_BODY()
public:
	virtual void InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner) override;
	virtual void Subscribe() override;
	virtual void Unsubscribe() override;
	virtual void HitTriggerCleanup(FHitInstance& Hit) override;

	virtual bool CanExecute() const override { return false; }

private:
	void OnBeingTargeted(FAttackContext& Context, FHitInstance& Hit);

	static constexpr int32 AccuracyPenalty = -20;

	// Stable per-instance ID used to add/remove the modifier on the attacker's Accuracy stat.
	FGuid ModifierID;
};
