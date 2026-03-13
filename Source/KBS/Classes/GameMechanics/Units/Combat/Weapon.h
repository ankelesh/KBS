#pragma once
#include "CoreMinimal.h"
#include "Weapon.generated.h"

class UWeaponDataAsset;
class UCombatDescriptor;

UCLASS(BlueprintType)
class KBS_API UWeapon : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(UObject* Outer, UWeaponDataAsset* Data);

	UCombatDescriptor* GetDescriptor() const { return Descriptor; }
	FText GetDisplayName() const;
	const FText& GetDescription() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UWeaponDataAsset> Config;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UCombatDescriptor> Descriptor;
};
