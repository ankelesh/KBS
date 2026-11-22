#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "AutoAttackAbility.generated.h"

class UCombatDescriptor;

UCLASS(Blueprintable)
class KBS_API UAutoAttackAbility : public UUnitAbility
{
protected:
	// Descriptor actually used for targeting/preview/execution given the one picked from the weapon.
	virtual UCombatDescriptor* ResolveAttackDescriptor(UCombatDescriptor* WeaponDescriptor) const { return WeaponDescriptor; }
	// Called after the attack resolved, with the descriptor that was used. Append any extra log steps to OutSteps.
	virtual void OnAttackResolved(const UCombatDescriptor* UsedDescriptor, TArray<TInstancedStruct<FTacLogStepBase>>& OutSteps) {}
	// Every descriptor this ability may attack with; ability tags are cached, so they must not depend on current state.
	virtual void GatherTagDescriptors(UCombatDescriptor* WeaponDescriptor, TArray<UCombatDescriptor*>& OutDescriptors) const
	{
		OutDescriptors.Add(WeaponDescriptor);
	}

public:
	virtual TMap<FTacCoordinates, FPreviewHitResult> DamagePreview(FTacCoordinates TargetCell) const override;

	virtual FAbilityExecutionResult Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override;
	virtual bool CanExecute() const override;
	

private:
	GENERATED_BODY()
	virtual EAbilityTurnReleasePolicy DecideTurnRelease() const override;
public:
	virtual FTargetingDescriptor GetTargeting() const override;
	virtual FGameplayTagContainer BuildTags() const override;
};

