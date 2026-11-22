#include "Misc/AutomationTest.h"
#include "GameMechanics/Units/Components/UnitChargeComponent.h"
#include "GameMechanics/Units/Components/Config/UnitChargeComponentConfig.h"
#include "GameplayTagsManager.h"

namespace
{
	FGameplayTag GunpowderTag() { return FGameplayTag::RequestGameplayTag(TEXT("Unit.Charge.Gunpowder")); }

	UUnitChargeComponent* MakeGunpowder(int32 Max, int32 Start = -1)
	{
		FUnitChargeComponentEntry Entry;
		Entry.Tag = GunpowderTag();
		Entry.DisplayName = FText::FromString(TEXT("Gunpowder"));
		Entry.CounterMax = Max;
		Entry.CounterStart = Start;

		FUnitChargeComponentConfig Config;
		Config.Entries.Add(Entry);

		UUnitChargeComponent* Component = NewObject<UUnitChargeComponent>(GetTransientPackage());
		Component->InitializeFromConfig(Config);
		return Component;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitChargeComponentLifecycleTest,
	"KBS.Units.ChargeComponent.Lifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitChargeComponentLifecycleTest::RunTest(const FString& Parameters)
{
	UUnitChargeComponent* Charges = MakeGunpowder(2);
	TestEqual("Starts at max when CounterStart is -1", Charges->GetCharge(GunpowderTag()), 2);
	TestEqual("Cap exposed", Charges->GetChargeCap(GunpowderTag()), 2);
	TestTrue("Full at start", Charges->IsFull(GunpowderTag()));

	Charges->DecrementCharge(GunpowderTag());
	TestEqual("Decrement", Charges->GetCharge(GunpowderTag()), 1);
	TestTrue("Can afford 1", Charges->HasCharge(GunpowderTag()));
	TestFalse("Can't afford 2", Charges->HasCharge(GunpowderTag(), 2));

	Charges->DecrementCharge(GunpowderTag(), 5);
	TestEqual("Decrement clamps at 0", Charges->GetCharge(GunpowderTag()), 0);
	TestFalse("Empty can't afford", Charges->HasCharge(GunpowderTag()));

	Charges->IncrementCharge(GunpowderTag(), 5);
	TestEqual("Increment clamps at cap", Charges->GetCharge(GunpowderTag()), 2);

	Charges->DropCharge(GunpowderTag());
	TestEqual("Drop", Charges->GetCharge(GunpowderTag()), 0);
	Charges->RefreshCharge(GunpowderTag());
	TestEqual("Refresh restores cap", Charges->GetCharge(GunpowderTag()), 2);

	UUnitChargeComponent* Partial = MakeGunpowder(3, 1);
	TestEqual("Explicit CounterStart", Partial->GetCharge(GunpowderTag()), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitChargeComponentMetadataTest,
	"KBS.Units.ChargeComponent.Metadata",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitChargeComponentMetadataTest::RunTest(const FString& Parameters)
{
	UUnitChargeComponent* Charges = MakeGunpowder(1);
	TestEqual("One tracked tag", Charges->GetTrackedTags().Num(), 1);
	TestEqual("Display name kept", Charges->GetChargeName(GunpowderTag()).ToString(), FString(TEXT("Gunpowder")));
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitChargeComponentQueryTest,
	"KBS.Units.ChargeComponent.Queries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitChargeComponentQueryTest::RunTest(const FString& Parameters)
{
	UUnitChargeComponent* Charges = MakeGunpowder(2);
	const FGameplayTag Gunpowder = GunpowderTag();

	TestTrue("Registered tag is tracked", Charges->IsTracked(Gunpowder));
	TestFalse("Unregistered tag is not tracked", Charges->IsTracked(FGameplayTag()));
	TestTrue("Full at start", Charges->IsFull(Gunpowder));
	TestTrue("Has 2 of 2", Charges->HasCharge(Gunpowder, 2));
	TestFalse("Does not have 3 of 2", Charges->HasCharge(Gunpowder, 3));

	Charges->DecrementCharge(Gunpowder);
	TestFalse("Not full after spending", Charges->IsFull(Gunpowder));
	TestTrue("Still has 1", Charges->HasCharge(Gunpowder));
	TestFalse("No longer has 2", Charges->HasCharge(Gunpowder, 2));

	Charges->DropCharge(Gunpowder);
	TestFalse("Empty has none", Charges->HasCharge(Gunpowder));
	return true;
}
