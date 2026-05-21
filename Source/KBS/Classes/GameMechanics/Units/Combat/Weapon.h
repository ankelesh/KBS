#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTypes/CombatDescriptorTypes.h"
#include "WeaponDataAsset.h"
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
	FText GetDisplayName() const;
	const FText& GetDescription() const;
	bool IsUsableForAutoAttack() const { return Config.Designation != ECombatDescriptorDesignation::Spells; }
	bool IsUsableForSpells() const { return Config.Designation != ECombatDescriptorDesignation::AutoAttacks; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	FWeaponData Config;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UCombatDescriptor> Descriptor;
};
