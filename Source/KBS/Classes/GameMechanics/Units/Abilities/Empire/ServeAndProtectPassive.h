#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "ServeAndProtectPassive.generated.h"

/**
 * Passive: a friendly unit standing in the cell directly behind the owner borrows the owner's armour while it is hit.
 * Per damage source the target's armour is raised to the owner's current (modified) armour, if lower.
 * Only armour is shared (not wards, damage reduction or defensive stance), and only on damaging hits.
 *
 * Lifecycle per hit:
 *   Subscribe()             - binds to the combat subsystem's calculation phase
 *   OnCalculationPhase()    - adds flat armour modifiers on the target; registers with Hit.Interfere()
 *   HitTriggerCleanup(Hit)  - removes them (called by ~FHitInstance)
 * Position is read at the moment of the hit, so cover ends by itself when either unit moves or dies.
 */
UCLASS()
class KBS_API UServeAndProtectPassive : public UUnitAbility
{
	GENERATED_BODY()
public:
	virtual void InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner) override;
	virtual void Subscribe() override;
	virtual void Unsubscribe() override;
	virtual void HitTriggerCleanup(FHitInstance& Hit) override;
	virtual bool CanExecute() const override { return false; }

private:
	UFUNCTION()
	void OnCalculationPhase(FCombatContext& Context, FHitInstance& Hit);

	bool IsStandingBehindOwner(const AUnit* Unit) const;

	// Stable per-instance ID used to add/remove the modifiers on the covered unit's armour.
	FGuid ModifierID;

	// Armour added per covered unit for the hit in flight, so cleanup removes exactly what was added.
	TMap<TWeakObjectPtr<AUnit>, TMap<EDamageSource, int32>> AppliedCover;
};
