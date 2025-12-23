#include "GameMechanics/Tactical/Grid/Components/AbilityExecutorComponent.h"
#include "GameMechanics/Tactical/Grid/Components/PresentationTrackerComponent.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Unit.h"
UAbilityExecutorComponent::UAbilityExecutorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UAbilityExecutorComponent::Initialize(ATacBattleGrid* InGrid, UPresentationTrackerComponent* InPresentationTracker)
{
	Grid = InGrid;
	PresentationTracker = InPresentationTracker;
}
FAbilityValidation UAbilityExecutorComponent::ValidateAbility(UUnitAbilityInstance* Ability, const FAbilityBattleContext& Context) const
{
	if (!Ability)
	{
		return FAbilityValidation::Failure(EAbilityFailureReason::Custom, FText::FromString("Ability is null"));
	}
	return Ability->CanExecute(Context);
}
FAbilityResult UAbilityExecutorComponent::ExecuteAbility(UUnitAbilityInstance* Ability, const FAbilityBattleContext& Context)
{
	FAbilityValidation Validation = ValidateAbility(Ability, Context);
	if (!Validation.bIsValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityExecutor: Ability validation failed - %s"),
			*Validation.FailureMessage.ToString());
		return FAbilityResult::Failure(Validation.FailureReason, Validation.FailureMessage);
	}
	FString AbilityName = Ability ? Ability->GetName() : TEXT("Unknown");
	if (PresentationTracker)
	{
		PresentationTracker->BeginBatch(FString::Printf(TEXT("Ability_%s"), *AbilityName));
	}
	FAbilityResult TriggerResult = Ability->TriggerAbility(Context);
	if (!TriggerResult.bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityExecutor: Ability trigger failed"));
		if (PresentationTracker)
		{
			PresentationTracker->EndBatch();
		}
		return TriggerResult;
	}
	FAbilityResult ApplyResult = Ability->ApplyAbilityEffect(Context);
	if (PresentationTracker)
	{
		PresentationTracker->EndBatch();
	}
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

FAbilityBattleContext UAbilityExecutorComponent::BuildContext(AUnit* SourceUnit, const TArray<AUnit*>& Targets, FIntPoint ClickedCell, uint8 ClickedLayer) const
{
	FAbilityBattleContext Context;
	Context.SourceUnit = SourceUnit;
	Context.TargetUnits = Targets;
	Context.Grid = Grid;
	Context.TargetCell = ClickedCell;
	Context.TargetLayer = ClickedLayer;
	return Context;
}

void UAbilityExecutorComponent::ResolveResult(const FAbilityResult& Result)
{
	if (Result.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("AbilityExecutor: Resolving successful ability - Turn action: %d"),
			static_cast<uint8>(Result.TurnAction));
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
	if (Result.EffectsApplied.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("  - Applied %d effects"), Result.EffectsApplied.Num());
	}
	if (Result.UnitsAffected.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("  - Affected %d units"), Result.UnitsAffected.Num());
	}
}
