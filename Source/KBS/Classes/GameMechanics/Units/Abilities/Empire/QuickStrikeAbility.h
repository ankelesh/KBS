#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/Defaults/AutoAttackAbility.h"
#include "QuickStrikeAbility.generated.h"

/**
 * Targeted attack for the Defend slot: hits for half the weapon's damage, then puts the owner into the
 * defensive stance. Assign it as the unit's DefaultDefendAbility.
 */
UCLASS()
class KBS_API UQuickStrikeAbility : public UAutoAttackAbility
{
	GENERATED_BODY()
public:
	virtual FGameplayTagContainer BuildTags() const override;

protected:
	virtual UCombatDescriptor* ResolveAttackDescriptor(UCombatDescriptor* WeaponDescriptor) const override;
	virtual void OnAttackResolved(const UCombatDescriptor* UsedDescriptor,
	                              TArray<TInstancedStruct<FTacLogStepBase>>& OutSteps) override;

private:
	static constexpr int32 DamagePercent = 50;

	UPROPERTY()
	TObjectPtr<UCombatDescriptor> ScaledDescriptor;
};
