#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "OffensiveSpellAbility.generated.h"

class UCombatDescriptor;

UCLASS(Blueprintable)
class KBS_API UOffensiveSpellAbility : public UUnitAbility
{
	GENERATED_BODY()
public:
	virtual void InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner) override;

	virtual TMap<FTacCoordinates, FPreviewHitResult> DamagePreview(FTacCoordinates TargetCell) const override;

	virtual FAbilityExecutionResult Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override;
	virtual bool CanExecute() const override;

	virtual FTargetingDescriptor GetTargeting() const override;
	virtual FGameplayTagContainer BuildTags() const override;

private:
	// Instantiated from SpellAbilityDefinition; base damage is scaled before each use
	UPROPERTY()
	TObjectPtr<UCombatDescriptor> EmbeddedDescriptor;

	void ScaleEmbeddedDescriptor() const;
};
