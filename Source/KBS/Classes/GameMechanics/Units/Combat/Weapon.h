#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTypes/CombatDescriptorTypes.h"
#include "WeaponDataAsset.h"
#include "GameMechanics/Units/Components/Config/UnitChargeCost.h"
#include "Weapon.generated.h"

class UCombatDescriptor;

UCLASS(BlueprintType)
class KBS_API UWeapon : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(UObject* Outer, const FWeaponData& Data);

	UCombatDescriptor* GetDescriptor() const { return Descriptor; }
	FGameplayTag GetAnimTag() const;
	FSoftObjectPath GetDeliveryManifestPath() const { return FSoftObjectPath(Config.DefaultDelivery); }
	FText GetDisplayName() const;
	const FText& GetDescription() const;
	bool IsUsableForAutoAttack() const { return Config.Designation != ECombatDescriptorDesignation::Spells; }
	bool IsUsableForSpells() const { return Config.Designation != ECombatDescriptorDesignation::AutoAttacks; }
	// What one attack costs; invalid Tag means free. The owning unit decides whether it can pay.
	FUnitChargeCost GetChargeCost() const { return FUnitChargeCost{Config.ChargeTag, Config.ChargeCost}; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	FWeaponData Config;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UCombatDescriptor> Descriptor;
};
