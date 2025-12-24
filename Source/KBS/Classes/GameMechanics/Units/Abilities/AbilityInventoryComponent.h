#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameMechanics/Units/Abilities/AbilityDisplayData.h"
#include "GameplayTypes/AbilityTypes.h"
#include "AbilityInventoryComponent.generated.h"
class UUnitAbilityInstance;
struct FAbilityDisplayData;
enum class EDefaultAbilitySlot : uint8;
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
	UFUNCTION(BlueprintCallable, Category = "Abilities|DefaultSlots")
	void SetDefaultAbility(EDefaultAbilitySlot Slot, UUnitAbilityInstance* Ability);
	UFUNCTION(BlueprintPure, Category = "Abilities|DefaultSlots")
	UUnitAbilityInstance* GetDefaultAbility(EDefaultAbilitySlot Slot) const;
	UFUNCTION(BlueprintPure, Category = "Abilities|DefaultSlots")
	TArray<UUnitAbilityInstance*> GetAllDefaultAbilities() const;
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void SelectAttackAbility();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UUnitAbilityInstance> CurrentActiveAbility;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots")
	TObjectPtr<UUnitAbilityInstance> DefaultAttackAbility;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots")
	TObjectPtr<UUnitAbilityInstance> DefaultMoveAbility;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots")
	TObjectPtr<UUnitAbilityInstance> DefaultWaitAbility;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots")
	TObjectPtr<UUnitAbilityInstance> DefaultDefendAbility;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots")
	TObjectPtr<UUnitAbilityInstance> DefaultFleeAbility;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UUnitAbilityInstance>> AvailableActiveAbilities;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UUnitAbilityInstance>> PassiveAbilities;
};
