#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "OffensiveSpellAbility.generated.h"

class UWeapon;

UCLASS(Blueprintable)
class KBS_API UOffensiveSpellAbility : public UUnitAbilityInstance
{
	GENERATED_BODY()
public:
	virtual void InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner) override;

	virtual TMap<FTacCoordinates, FPreviewHitResult> DamagePreview(FTacCoordinates TargetCell) const override;

	virtual bool Execute(FTacCoordinates TargetCell) override;
	virtual bool CanExecute(FTacCoordinates TargetCell) const override;
	virtual bool CanExecute() const override;

	virtual ETargetReach GetTargeting() const override;

private:
	// Instantiated from SpellAbilityDefinition::EmbeddedWeaponAsset; base damage is scaled before each use
	UPROPERTY()
	TObjectPtr<UWeapon> EmbeddedWeapon;

	void ScaleEmbeddedWeapon() const;
};
