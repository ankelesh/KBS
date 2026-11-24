#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "HistoryActionPassive.generated.h"

class UHistoryActionPassiveDefinition;

/**
 * Passive that fires OnTriggered() when the owner's action history records the configured action
 * (optionally only when it differs from the previous one). Subclasses only implement the reaction.
 */
UCLASS(Abstract)
class KBS_API UHistoryActionPassive : public UUnitAbility
{
	GENERATED_BODY()
public:
	virtual void Subscribe() override;
	virtual void Unsubscribe() override;
	virtual bool CanExecute() const override { return false; }

protected:
	virtual void OnTriggered() {}
	const UHistoryActionPassiveDefinition& GetTriggerConfig() const;
	// Records the steps as a Reaction event under the action that triggered it. No-op for empty steps.
	void LogReaction(TArray<TInstancedStruct<FTacLogStepBase>>&& Steps) const;

private:
	UFUNCTION()
	void HandleActionRecorded(const FGameplayTag& ActionTag, bool bChangedFromPrevious);
};

UCLASS()
class KBS_API UHistoryActionModifyStatsPassive : public UHistoryActionPassive
{
	GENERATED_BODY()
protected:
	virtual void OnTriggered() override;
};

UCLASS()
class KBS_API UHistoryActionApplyEffectPassive : public UHistoryActionPassive
{
	GENERATED_BODY()
protected:
	virtual void OnTriggered() override;
};
