// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTypes/AbilityTypes.h"
#include "GameplayTypes/AbilityBattleContext.h"
#include "AbilityExecutorComponent.generated.h"

class ATacBattleGrid;
class UUnitAbilityInstance;
class UPresentationTrackerComponent;
class AUnit;

/**
 * Manages ability validation, execution, and result resolution
 * Single responsibility for all ability lifecycle operations
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KBS_API UAbilityExecutorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAbilityExecutorComponent();

	/**
	 * Initialize with grid reference
	 */
	void Initialize(ATacBattleGrid* InGrid);

	/**
	 * Check if ability can execute in given context
	 */
	FAbilityValidation ValidateAbility(UUnitAbilityInstance* Ability, const FAbilityBattleContext& Context) const;

	/**
	 * Execute ability and return result object
	 */
	FAbilityResult ExecuteAbility(UUnitAbilityInstance* Ability, const FAbilityBattleContext& Context);

	/**
	 * Construct context object with grid and system references
	 */
	FAbilityBattleContext BuildContext(AUnit* SourceUnit, const TArray<AUnit*>& Targets) const;

	/**
	 * Handle result - broadcast events, manage turn flow
	 */
	void ResolveResult(const FAbilityResult& Result);

private:
	UPROPERTY()
	TObjectPtr<ATacBattleGrid> Grid;

	UPROPERTY()
	TObjectPtr<UPresentationTrackerComponent> PresentationTracker;
};
