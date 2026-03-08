#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameMechanics/Units/Abilities/AbilityDisplayData.h"
#include "GameplayTypes/AbilityTypes.h"
#include "AbilityInventoryComponent.generated.h"
class UUnitAbilityInstance;
class UUnitDefinition;
class AUnit;
struct FAbilityDisplayData;
struct FUnitStatusContainer;
enum class EDefaultAbilitySlot : uint8;


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KBS_API UAbilityInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Lifecycle
	UAbilityInventoryComponent();
	virtual void BeginPlay() override;
	void InitializeFromDefinition(const UUnitDefinition* Definition, AUnit* OwnerUnit);
	void RegisterPassives();
	void UnregisterPassives();
	
	
	// Add-remove
	void AddActiveAbility(UUnitAbilityInstance* Ability);
	void AddPassiveAbility(UUnitAbilityInstance* Ability);
	void RemovePassiveAbility(UUnitAbilityInstance* Ability);
	UFUNCTION(BlueprintCallable, Category = "Abilities|DefaultSlots")
	void SetDefaultAbility(EDefaultAbilitySlot Slot, UUnitAbilityInstance* Ability);
	UFUNCTION(BlueprintCallable, Category = "Abilities|Spellbook")
	void AddSpellbookAbility(UUnitAbilityInstance* Ability);
	
	
	// Getters
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
	UFUNCTION(BlueprintPure, Category = "Abilities|DefaultSlots")
	UUnitAbilityInstance* GetDefaultAbility(EDefaultAbilitySlot Slot) const;
	UFUNCTION(BlueprintPure, Category = "Abilities|Spellbook")
	TArray<UUnitAbilityInstance*> GetSpellbookAbilities() const;
	
	// Interface
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void EquipAbility(UUnitAbilityInstance* Ability);
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void EnsureValidAbility();
	UFUNCTION(BlueprintPure, Category = "Abilities")
	bool HasAnyAbilityAvailable() const;
	void RecheckContents();
	UFUNCTION(BlueprintCallable, Category = "Abilities|Spellbook|Display")
	bool IsSpellbookAvailable() const;
	FAbilityContext* GetContext();
	void ProcessTurnPolicy(EAbilityTurnReleasePolicy Policy);
	

	// DisplayData getters
	UFUNCTION(BlueprintCallable, Category = "Abilities|Spellbook|Display")
	TArray<FAbilityDisplayData> GetSpellbookDisplayData() const;
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities|Spellbook")
	TArray<TObjectPtr<UUnitAbilityInstance>> SpellbookAbilities;

private:

	FAbilityContext AbilityContext;
	bool IsDefaultAbility(UUnitAbilityInstance* Ability) const;
	bool IsAbilityAvailable(UUnitAbilityInstance* Ability) const;
	const FUnitStatusContainer* GetOwnerStatus() const;
	UFUNCTION() void OnOwnerTurnStart(AUnit* Unit);
	UFUNCTION() void OnOwnerTurnEnd(AUnit* Unit);
};
