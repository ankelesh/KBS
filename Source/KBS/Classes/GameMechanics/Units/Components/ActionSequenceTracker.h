#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

// Pure bookkeeping for UUnitActionHistoryComponent: previous action + progress through an ordered sequence.
// A step advances only when the recorded action matches the next expected one; other actions never reset it.
struct FActionSequenceTracker
{
	struct FResult
	{
		bool bChangedFromPrevious = false;
		bool bAdvanced = false;
		bool bCompleted = false;
	};

	void Init(TArray<FGameplayTag> InSequence)
	{
		Sequence = MoveTemp(InSequence);
		Previous = FGameplayTag();
		Step = 0;
	}

	FResult Record(const FGameplayTag& Action)
	{
		FResult Result;
		Result.bChangedFromPrevious = Previous.IsValid() && Previous != Action;
		Previous = Action;

		if (!IsComplete() && Sequence.IsValidIndex(Step) && Sequence[Step] == Action)
		{
			++Step;
			Result.bAdvanced = true;
			Result.bCompleted = IsComplete();
		}
		return Result;
	}

	bool IsComplete() const { return Sequence.Num() > 0 && Step == Sequence.Num(); }
	void ResetSequence() { Step = 0; }
	int32 GetStep() const { return Step; }
	int32 GetLength() const { return Sequence.Num(); }
	const FGameplayTag& GetPrevious() const { return Previous; }

private:
	TArray<FGameplayTag> Sequence;
	FGameplayTag Previous;
	int32 Step = 0;
};
