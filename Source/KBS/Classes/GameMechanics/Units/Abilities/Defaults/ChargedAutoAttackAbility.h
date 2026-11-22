#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/Defaults/AutoAttackAbility.h"
#include "ChargedAutoAttackAbility.generated.h"

class UCombatDescriptor;
class UChargedAutoAttackAbilityDefinition;

// Auto-attack that swaps the weapon descriptor for a charged one while the owner can pay its charge cost.
// Without charges it degrades to a plain weapon attack and pays nothing.
UCLASS(Blueprintable)
class KBS_API UChargedAutoAttackAbility : public UAutoAttackAbility
{
	GENERATED_BODY()
public:
	virtual void InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner) override;

protected:
	virtual UCombatDescriptor* ResolveAttackDescriptor(UCombatDescriptor* WeaponDescriptor) const override;
	virtual void OnAttackResolved(const UCombatDescriptor* UsedDescriptor,
	                              TArray<TInstancedStruct<FTacLogStepBase>>& OutSteps) override;
	virtual void GatherTagDescriptors(UCombatDescriptor* WeaponDescriptor,
	                                  TArray<UCombatDescriptor*>& OutDescriptors) const override;

private:
	const UChargedAutoAttackAbilityDefinition& GetChargedConfig() const;
	bool CanAffordCharge() const;

	UPROPERTY()
	TObjectPtr<UCombatDescriptor> ChargedDescriptor;
};
