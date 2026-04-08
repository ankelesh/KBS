#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameMechanics/Units/Stats/UnitStats.h"
#include "GameMechanics/Units/Components/UnitComponentEntry.h"
#include "GameMechanics/Units/Components/Config/UnitVisualDefinition.h"
#include "UnitDefinition.generated.h"

class UWeaponDataAsset;
class UUnitAbilityDefinition;

UCLASS(BlueprintType)
class KBS_API UUnitDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UUnitVisualDefinition> VisualDefinition;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString UnitName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FUnitCoreStats BaseStatsTemplate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<TObjectPtr<UWeaponDataAsset>> DefaultWeapons;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots", meta = (DisplayName = "Attack"))
	TObjectPtr<UUnitAbilityDefinition> DefaultAttackAbility;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots", meta = (DisplayName = "Move"))
	TObjectPtr<UUnitAbilityDefinition> DefaultMoveAbility;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots", meta = (DisplayName = "Wait"))
	TObjectPtr<UUnitAbilityDefinition> DefaultWaitAbility;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots", meta = (DisplayName = "Defend"))
	TObjectPtr<UUnitAbilityDefinition> DefaultDefendAbility;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities|DefaultSlots", meta = (DisplayName = "Flee"))
	TObjectPtr<UUnitAbilityDefinition> DefaultFleeAbility;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UUnitAbilityDefinition>> AdditionalAbilities;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities|Spellbook")
	TArray<TObjectPtr<UUnitAbilityDefinition>> SpellbookAbilities;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MovementSpeed = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	int32 UnitSize = 1; // 1 = single cell, 2 = two cells
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	int32 FlankEntranceArrivalDelay = 3; // turns unit must wait in a flank entrance before it can act; 0 = no delay
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	int32 FlankRearArrivalDelay = 3; // turns unit must wait in a flank rear before it can act; 0 = no delay
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<FUnitComponentEntry> ComponentEntries;
};
