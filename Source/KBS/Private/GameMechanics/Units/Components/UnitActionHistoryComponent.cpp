#include "GameMechanics/Units/Components/UnitActionHistoryComponent.h"
#include "GameMechanics/Units/Components/Config/UnitActionHistoryCmpConfig.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "GameMechanics/Units/Unit.h"

void UUnitActionHistoryComponent::BeginPlay()
{
	Super::BeginPlay();
	AUnit* OwnerUnit = Cast<AUnit>(GetOwner());
	checkf(OwnerUnit, TEXT("UUnitActionHistoryComponent must be attached to AUnit"));
	OwnerUnit->OnUnitAbilityUsed.AddDynamic(this, &UUnitActionHistoryComponent::OnAbilityUsed);
}

void UUnitActionHistoryComponent::InitializeFromConfig(const FUnitActionHistoryCmpConfig& Config)
{
	for (const FGameplayTag& Step : Config.TargetSequence)
	{
		checkf(Config.TrackedActions.Contains(Step), TEXT("Action history: sequence step '%s' is not a tracked action"),
		       *Step.ToString());
	}
	TrackedActions = Config.TrackedActions;
	Tracker.Init(Config.TargetSequence);
}

void UUnitActionHistoryComponent::OnAbilityUsed(AUnit* Unit, UUnitAbility* Ability)
{
	if (Unit->IsDead())
		return;

	const FGameplayTagContainer AbilityTags = Ability->GetTags();
	const FGameplayTag* Action = TrackedActions.FindByPredicate([&AbilityTags](const FGameplayTag& Tracked)
	{
		return AbilityTags.HasTag(Tracked);
	});
	if (!Action)
		return;

	const FActionSequenceTracker::FResult Result = Tracker.Record(*Action);
	OnActionRecorded.Broadcast(*Action, Result.bChangedFromPrevious);
	if (Result.bCompleted)
		OnSequenceCompleted.Broadcast();
}
