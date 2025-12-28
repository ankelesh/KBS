#include "Misc/AutomationTest.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameplayTypes/DamageTypes.h"

// Helper class for creating test fixtures
class FWeaponTestHelper
{
public:
	static UWeaponDataAsset* CreateTestWeaponData(UObject* Outer, int32 BaseDamage = 50, float AccuracyMultiplier = 1.0f)
	{
		UWeaponDataAsset* WeaponData = NewObject<UWeaponDataAsset>(Outer);
		WeaponData->Name = FText::FromString(TEXT("TestWeapon"));
		WeaponData->Description = FText::FromString(TEXT("A test weapon"));
		WeaponData->BaseStats.BaseDamage = BaseDamage;
		WeaponData->BaseStats.AccuracyMultiplier = AccuracyMultiplier;
		WeaponData->BaseStats.DamageSources.Add(EDamageSource::Physical);
		WeaponData->BaseStats.TargetReach = ETargetReach::AnyEnemy;
		return WeaponData;
	}

	static UWeapon* CreateTestWeapon(UObject* Outer)
	{
		return NewObject<UWeapon>(Outer);
	}
};

// Test: Initialize with valid data sets config and initializes stats
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponInitializeWithValidDataTest,
	"KBS.Units.Weapons.Initialize_WithValidData_SetsConfigAndInitializesStats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWeaponInitializeWithValidDataTest::RunTest(const FString& Parameters)
{
	UWeapon* Weapon = FWeaponTestHelper::CreateTestWeapon(GetTransientPackage());
	UWeaponDataAsset* WeaponData = FWeaponTestHelper::CreateTestWeaponData(GetTransientPackage(), 75, 1.2f);

	Weapon->Initialize(nullptr, WeaponData);

	TestNotNull("Config should be set", Weapon->GetConfig().Get());
	TestEqual("Base damage should match", Weapon->GetStats().BaseDamage, 75);
	TestEqual("Accuracy multiplier should match", Weapon->GetStats().AccuracyMultiplier, 1.2f);
	TestTrue("Should have Physical damage source", Weapon->GetStats().DamageSources.Contains(EDamageSource::Physical));
	TestEqual("Target reach should match", Weapon->GetStats().TargetReach, ETargetReach::AnyEnemy);

	return true;
}

// Test: GetStats returns modified stats
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponGetStatsReturnsModifiedStatsTest,
	"KBS.Units.Weapons.GetStats_ReturnsModifiedStats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWeaponGetStatsReturnsModifiedStatsTest::RunTest(const FString& Parameters)
{
	UWeapon* Weapon = FWeaponTestHelper::CreateTestWeapon(GetTransientPackage());
	UWeaponDataAsset* WeaponData = FWeaponTestHelper::CreateTestWeaponData(GetTransientPackage(), 100, 0.8f);

	Weapon->Initialize(nullptr, WeaponData);
	const FWeaponStats& Stats = Weapon->GetStats();

	TestEqual("Should return modified base damage", Stats.BaseDamage, 100);
	TestEqual("Should return modified accuracy", Stats.AccuracyMultiplier, 0.8f);

	return true;
}

// Test: GetEffects returns active effects
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponGetEffectsReturnsActiveEffectsTest,
	"KBS.Units.Weapons.GetEffects_ReturnsActiveEffects",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWeaponGetEffectsReturnsActiveEffectsTest::RunTest(const FString& Parameters)
{
	UWeapon* Weapon = FWeaponTestHelper::CreateTestWeapon(GetTransientPackage());
	UWeaponDataAsset* WeaponData = FWeaponTestHelper::CreateTestWeaponData(GetTransientPackage());

	Weapon->Initialize(nullptr, WeaponData);
	const TArray<TObjectPtr<UBattleEffect>>& Effects = Weapon->GetEffects();

	TestNotNull("Effects array should be accessible", &Effects);

	return true;
}

// Test: GetReach returns correct target reach
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponGetReachReturnsCorrectTargetReachTest,
	"KBS.Units.Weapons.GetReach_ReturnsCorrectTargetReach",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWeaponGetReachReturnsCorrectTargetReachTest::RunTest(const FString& Parameters)
{
	UWeapon* Weapon = FWeaponTestHelper::CreateTestWeapon(GetTransientPackage());
	UWeaponDataAsset* WeaponData = FWeaponTestHelper::CreateTestWeaponData(GetTransientPackage());
	WeaponData->BaseStats.TargetReach = ETargetReach::ClosestEnemies;

	Weapon->Initialize(nullptr, WeaponData);

	TestEqual("Reach should match data asset", Weapon->GetReach(), ETargetReach::ClosestEnemies);

	return true;
}

// Test: RecalculateModifiedStats updates stats from base
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponRecalculateModifiedStatsTest,
	"KBS.Units.Weapons.RecalculateModifiedStats_UpdatesStatsFromBase",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWeaponRecalculateModifiedStatsTest::RunTest(const FString& Parameters)
{
	UWeapon* Weapon = FWeaponTestHelper::CreateTestWeapon(GetTransientPackage());
	UWeaponDataAsset* WeaponData = FWeaponTestHelper::CreateTestWeaponData(GetTransientPackage(), 50, 1.0f);

	Weapon->Initialize(nullptr, WeaponData);
	Weapon->RecalculateModifiedStats();

	TestEqual("Modified damage should equal base", Weapon->GetStats().BaseDamage, 50);
	TestEqual("Modified accuracy should equal base", Weapon->GetStats().AccuracyMultiplier, 1.0f);

	return true;
}

// Test: Restore resets to base stats
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponRestoreResetsToBaseStatsTest,
	"KBS.Units.Weapons.Restore_ResetsToBaseStats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWeaponRestoreResetsToBaseStatsTest::RunTest(const FString& Parameters)
{
	UWeapon* Weapon = FWeaponTestHelper::CreateTestWeapon(GetTransientPackage());
	UWeaponDataAsset* WeaponData = FWeaponTestHelper::CreateTestWeaponData(GetTransientPackage(), 60, 0.9f);

	Weapon->Initialize(nullptr, WeaponData);
	Weapon->Restore();

	TestEqual("Damage should be restored to base", Weapon->GetStats().BaseDamage, 60);
	TestEqual("Accuracy should be restored to base", Weapon->GetStats().AccuracyMultiplier, 0.9f);

	return true;
}

// Test: Initialize with null data handles gracefully
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponInitializeWithNullDataTest,
	"KBS.Units.Weapons.Initialize_WithNullData_HandlesGracefully",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWeaponInitializeWithNullDataTest::RunTest(const FString& Parameters)
{
	UWeapon* Weapon = FWeaponTestHelper::CreateTestWeapon(GetTransientPackage());

	AddExpectedError(TEXT("UWeapon::Initialize - No WeaponDataAsset provided"), EAutomationExpectedErrorFlags::Contains, 1);
	Weapon->Initialize(nullptr, nullptr);

	TestNull("Config should remain null", Weapon->GetConfig().Get());

	return true;
}
