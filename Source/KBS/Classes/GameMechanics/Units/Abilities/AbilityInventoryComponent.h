#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameMechanics/Units/Abilities/AbilityDisplayData.h"
#include "GameplayTypes/AbilityTypes.h"
#include "AbilityInventoryComponent.generated.h"
class UUnitAbility;
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
	void AddOptionalAbility(UUnitAbility* Ability);
	void AddPassiveAbility(UUnitAbility* Ability);
	void RemovePassiveAbility(UUnitAbility* Ability);
	void AssignDefaultAbility(EDefaultAbilitySlot Slot, UUnitAbility* Ability);
	UFUNCTION(BlueprintCallable, Category = "Abilities|Spellbook")
	void AddSpellbookAbility(UUnitAbility* Ability);
	
	
	// Getters
	UFUNCTION(BlueprintPure, Category = "Abilities")
	UUnitAbility* GetCurrentActiveAbility() const;
	UFUNCTION(BlueprintPure, Category = "Abilities")
	TArray<UUnitAbility*> GetAvailableActiveAbilities() const;
	UFUNCTION(BlueprintPure, Category = "Abilities")
	TArray<UUnitAbility*> GetPassiveAbilities() const;
	UFUNCTION(BlueprintCallable, Category = "Abilities|Display")
	TArray<FAbilityDisplayData> GetActiveAbilitiesDisplayData() const;
	UFUNCTION(BlueprintCallable, Category = "Abilities|Display")
	TArray<FAbilityDisplayData> GetPassiveAbilitiesDisplayData() const;
	UFUNCTION(BlueprintPure, Category = "Abilities|DefaultSlots")
	UUnitAbility* GetDefaultAbility(EDefaultAbilitySlot Slot) const;
	UFUNCTION(BlueprintPure, Category = "Abilities|Spellbook")
	TArray<UUnitAbility*> GetSpellbookAbilities() const;
	
	// Interface
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void EquipAbility(UUnitAbility* Ability);
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void EnsureValidAbility();
	UFUNCTION(BlueprintPure, Category = "Abilities")
	bool HasAnyAbilityAvailable() const;
	void RecheckContents();
	UFUNCTION(BlueprintCallable, Category = "Abilities|Spellbook|Display")
	bool IsSpellbookAvailable() const;
	FAbilityContext* GetContext();
	void ProcessTurnPolicy(EAbilityTurnReleasePolicy Policy);
	FString GetAbilitiesDebugString() const;
	

	// DisplayData getters
	UFUNCTION(BlueprintCallable, Category = "Abilities|Spellbook|Display")
	TArray<FAbilityDisplayData> GetSpellbookDisplayData() const;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UUnitAbility> CurrentActiveAbility;
	// Indexed by EDefaultAbilitySlot; every slot is filled during InitializeFromDefinition
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots")
	TArray<TObjectPtr<UUnitAbility>> DefaultAbilities;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UUnitAbility>> AvailableActiveAbilities;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UUnitAbility>> PassiveAbilities;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities|Spellbook")
	TArray<TObjectPtr<UUnitAbility>> SpellbookAbilities;

private:

	FAbilityContext AbilityContext;
	bool IsDefaultAbility(UUnitAbility* Ability) const;
	bool IsAbilityAvailable(UUnitAbility* Ability) const;
	const FUnitStatusContainer* GetOwnerStatus() const;
	UFUNCTION() void OnOwnerTurnStart(AUnit* Unit);
	UFUNCTION() void OnOwnerTurnEnd(AUnit* Unit);
};
