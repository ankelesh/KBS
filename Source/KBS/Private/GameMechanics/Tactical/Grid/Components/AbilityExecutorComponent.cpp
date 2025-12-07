// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMechanics/Tactical/Grid/Components/AbilityExecutorComponent.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Unit.h"

UAbilityExecutorComponent::UAbilityExecutorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAbilityExecutorComponent::Initialize(ATacBattleGrid* InGrid)
{
	Grid = InGrid;
}

FAbilityValidation UAbilityExecutorComponent::ValidateAbility(UUnitAbilityInstance* Ability, const FAbilityBattleContext& Context) const
{
	if (!Ability)
	{
		return FAbilityValidation::Failure(EAbilityFailureReason::Custom, FText::FromString("Ability is null"));
	}

	// Call ability's own validation
	return Ability->CanExecute(Context);
}

FAbilityResult UAbilityExecutorComponent::ExecuteAbility(UUnitAbilityInstance* Ability, const FAbilityBattleContext& Context)
{
	// Validate first
	FAbilityValidation Validation = ValidateAbility(Ability, Context);
	if (!Validation.bIsValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityExecutor: Ability validation failed - %s"),
			*Validation.FailureMessage.ToString());
		return FAbilityResult::Failure(Validation.FailureReason, Validation.FailureMessage);
	}

	// Trigger ability
	FAbilityResult TriggerResult = Ability->TriggerAbility(Context);
	if (!TriggerResult.bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityExecutor: Ability trigger failed"));
		return TriggerResult;
	}

	// Apply ability effects
	FAbilityResult ApplyResult = Ability->ApplyAbilityEffect(Context);

	UE_LOG(LogTemp, Log, TEXT("AbilityExecutor: Ability executed successfully, affected %d units"),
		ApplyResult.UnitsAffected.Num());

	return ApplyResult;
}

FAbilityBattleContext UAbilityExecutorComponent::BuildContext(AUnit* SourceUnit, const TArray<AUnit*>& Targets) const
{
	FAbilityBattleContext Context;
	Context.SourceUnit = SourceUnit;
	Context.TargetUnits = Targets;
	Context.Grid = Grid;
	return Context;
}

void UAbilityExecutorComponent::ResolveResult(const FAbilityResult& Result)
{
	if (Result.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("AbilityExecutor: Resolving successful ability - Turn action: %d"),
			static_cast<uint8>(Result.TurnAction));

		// Broadcast completion event
		if (Grid)
		{
			Grid->OnAbilityComplete.Broadcast();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityExecutor: Ability failed - %s"),
			*Result.FailureMessage.ToString());
	}

	// Log effects and units affected
	if (Result.EffectsApplied.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("  - Applied %d effects"), Result.EffectsApplied.Num());
	}

	if (Result.UnitsAffected.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("  - Affected %d units"), Result.UnitsAffected.Num());
	}
}
