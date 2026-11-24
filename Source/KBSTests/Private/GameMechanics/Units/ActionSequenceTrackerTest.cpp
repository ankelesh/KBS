#include "Misc/AutomationTest.h"
#include "GameMechanics/Units/Components/ActionSequenceTracker.h"
#include "GameplayTagsManager.h"

namespace
{
	FGameplayTag Attack() { return FGameplayTag::RequestGameplayTag(TEXT("Ability.AutoAttack")); }
	FGameplayTag Defend() { return FGameplayTag::RequestGameplayTag(TEXT("Ability.Defend")); }
	FGameplayTag Wait() { return FGameplayTag::RequestGameplayTag(TEXT("Ability.Wait")); }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FActionSequenceTrackerChangeTest,
	"KBS.Units.ActionSequenceTracker.ChangeDetection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FActionSequenceTrackerChangeTest::RunTest(const FString& Parameters)
{
	FActionSequenceTracker Tracker;
	Tracker.Init({});

	TestFalse("First action is not a change", Tracker.Record(Attack()).bChangedFromPrevious);
	TestFalse("Repeat is not a change", Tracker.Record(Attack()).bChangedFromPrevious);
	TestTrue("Different action is a change", Tracker.Record(Defend()).bChangedFromPrevious);
	TestEqual("Previous tracked", Tracker.GetPrevious(), Defend());
	TestFalse("Empty sequence never completes", Tracker.IsComplete());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FActionSequenceTrackerSequenceTest,
	"KBS.Units.ActionSequenceTracker.Sequence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FActionSequenceTrackerSequenceTest::RunTest(const FString& Parameters)
{
	FActionSequenceTracker Tracker;
	Tracker.Init({Attack(), Defend(), Wait()});

	TestTrue("Matching step advances", Tracker.Record(Attack()).bAdvanced);
	TestFalse("Out-of-order action does not advance", Tracker.Record(Wait()).bAdvanced);
	TestEqual("Out-of-order action does not reset", Tracker.GetStep(), 1);
	TestTrue("Next expected step advances", Tracker.Record(Defend()).bAdvanced);

	const FActionSequenceTracker::FResult Last = Tracker.Record(Wait());
	TestTrue("Last step completes", Last.bCompleted);
	TestTrue("Tracker complete", Tracker.IsComplete());

	TestFalse("Completed sequence ignores further actions", Tracker.Record(Attack()).bAdvanced);

	Tracker.ResetSequence();
	TestFalse("Reset clears completion", Tracker.IsComplete());
	TestEqual("Reset clears step", Tracker.GetStep(), 0);
	TestEqual("Reset keeps previous action", Tracker.GetPrevious(), Attack());
	return true;
}
