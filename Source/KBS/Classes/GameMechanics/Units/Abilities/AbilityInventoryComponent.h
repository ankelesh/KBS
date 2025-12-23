#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameMechanics/Units/Abilities/AbilityDisplayData.h"
#include "AbilityInventoryComponent.generated.h"
class UUnitAbilityInstance;
struct FAbilityDisplayData;
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KBS_API UAbilityInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UAbilityInventoryComponent();
	void AddActiveAbility(UUnitAbilityInstance* Ability);
	void AddPassiveAbility(UUnitAbilityInstance* Ability);
	void RemovePassiveAbility(UUnitAbilityInstance* Ability);
	void RegisterPassives();
	void UnregisterPassives();
	UFUNCTION(BlueprintPure, Category = "Abilities")
	UUnitAbilityInstance* GetCurrentActiveAbility() const;
	UFUNCTION(BlueprintPure, Category = "Abilities")
	TArray<UUnitAbilityInstance*> GetAvailableActiveAbilities() const;
	UFUNCTION(BlueprintPure, Category = "Abilities")
	TArray<UUnitAbilityInstance*> GetPassiveAbilities() const;
	UFUNCTION(BlueprintCallable, Category = "Abilities|Display")
	TArray<FAbilityDisplayData> GetActiveAbilitiesDisplayData() const;
	UFUNCTION(BlueprintCallable, Category = "Abilities|Display")
	TArray<FAbilityDisplayData> GetPassiveAbilitiesDisplayData() const;
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void EquipAbility(UUnitAbilityInstance* Ability);
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void EquipDefaultAbility();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UUnitAbilityInstance> CurrentActiveAbility;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UUnitAbilityInstance>> AvailableActiveAbilities;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UUnitAbilityInstance>> PassiveAbilities;
};
