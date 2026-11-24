#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameMechanics/Units/Components/ActionSequenceTracker.h"
#include "UnitActionHistoryComponent.generated.h"

struct FUnitActionHistoryCmpConfig;
class AUnit;
class UUnitAbility;

// Fires for every tracked action. bChangedFromPrevious is false for the unit's first tracked action.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionRecorded, const FGameplayTag&, ActionTag, bool, bChangedFromPrevious);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionSequenceCompleted);

/**
 * Records which tracked action (by tag) the owner used last and how far it got through a configured sequence.
 * Holds no gameplay logic: abilities subscribe to the delegates and decide what an action is worth.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KBS_API UUnitActionHistoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	void InitializeFromConfig(const FUnitActionHistoryCmpConfig& Config);

	bool IsTrackedAction(const FGameplayTag& Action) const { return TrackedActions.Contains(Action); }
	const FGameplayTag& GetPreviousAction() const { return Tracker.GetPrevious(); }
	bool IsSequenceComplete() const { return Tracker.IsComplete(); }
	int32 GetSequenceStep() const { return Tracker.GetStep(); }
	int32 GetSequenceLength() const { return Tracker.GetLength(); }
	void ResetSequence() { Tracker.ResetSequence(); }

	UPROPERTY(BlueprintAssignable, Category = "History")
	FOnActionRecorded OnActionRecorded;
	UPROPERTY(BlueprintAssignable, Category = "History")
	FOnActionSequenceCompleted OnSequenceCompleted;

private:
	UFUNCTION()
	void OnAbilityUsed(AUnit* Unit, UUnitAbility* Ability);

	// Abilities carrying none of these tags (e.g. movement) are invisible to the history.
	TArray<FGameplayTag> TrackedActions;
	FActionSequenceTracker Tracker;
};
