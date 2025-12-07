// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilityInventoryComponent.generated.h"

class UUnitAbilityInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityEquipped, UUnitAbilityInstance*, Ability);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KBS_API UAbilityInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAbilityInventoryComponent();

	void EquipAbility(UUnitAbilityInstance* Ability);
	void AddPassiveAbility(UUnitAbilityInstance* Ability);
	void RemovePassiveAbility(UUnitAbilityInstance* Ability);
	void RegisterPassives();
	void UnregisterPassives();

	UUnitAbilityInstance* GetCurrentActiveAbility() const { return CurrentActiveAbility; }
	const TArray<TObjectPtr<UUnitAbilityInstance>>& GetAvailableActiveAbilities() const { return AvailableActiveAbilities; }
	const TArray<TObjectPtr<UUnitAbilityInstance>>& GetPassiveAbilities() const { return PassiveAbilities; }

	UPROPERTY(BlueprintAssignable, Category = "Abilities")
	FOnAbilityEquipped OnAbilityEquipped;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UUnitAbilityInstance> CurrentActiveAbility;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UUnitAbilityInstance>> AvailableActiveAbilities;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UUnitAbilityInstance>> PassiveAbilities;
};
