#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "ArmourPenetrationPassive.generated.h"

/**
 * Passive: while the owner attacks, each target's Physical armour counts as 50% lower.
 *
 * Lifecycle per hit:
 *   Subscribe()             — binds to Owner->OnResolutionCalculating (attacker side)
 *   OnCalculating()         — adds a -50% Physical armour multiplier on the target; registers with Hit.Interfere()
 *   HitTriggerCleanup(Hit)  — removes the multiplier (called by ~FHitInstance)
 */
UCLASS()
class KBS_API UArmourPenetrationPassive : public UUnitAbility
{
	GENERATED_BODY()
public:
	virtual void InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner) override;
	virtual void Subscribe() override;
	virtual void Unsubscribe() override;
	virtual void HitTriggerCleanup(FHitInstance& Hit) override;
	virtual bool CanExecute() const override { return false; }

private:
	void OnCalculating(FCombatContext& Context, FHitInstance& Hit);

	static constexpr int32 ArmourMultiplierPercent = -50;

	FGuid ModifierID;
};
