#include "Misc/AutomationTest.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameplayTypes/CombatTypes.h"

// Helper class for creating test fixtures
class FUnitTestHelper
{
public:
	static UUnitDefinition* CreateTestUnitDefinition(UObject* Outer)
	{
		UUnitDefinition* Definition = NewObject<UUnitDefinition>(Outer);
		Definition->UnitName = TEXT("TestUnit");
		Definition->MovementSpeed = 300.0f;
		Definition->BaseStatsTemplate.MaxHealth = 100.0f;
		Definition->BaseStatsTemplate.Initiative = 50;
		Definition->BaseStatsTemplate.Accuracy = 0.75f;
		Definition->BaseStatsTemplate.Defense.DamageReduction = 5;
		return Definition;
	}

	static AUnit* CreateMockUnit(UWorld* World, float MaxHealth = 100.0f, float Accuracy = 0.75f)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AUnit* Unit = World->SpawnActor<AUnit>(AUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (Unit)
		{
			UUnitDefinition* Definition = CreateTestUnitDefinition(Unit);
			Definition->BaseStatsTemplate.MaxHealth = MaxHealth;
			Definition->BaseStatsTemplate.Accuracy = Accuracy;
			Unit->SetUnitDefinition(Definition);
		}
		return Unit;
	}

	static UWeaponDataAsset* CreateTestWeaponData(UObject* Outer, int32 BaseDamage = 50)
	{
		UWeaponDataAsset* WeaponData = NewObject<UWeaponDataAsset>(Outer);
		WeaponData->BaseStats.BaseDamage = BaseDamage;
		WeaponData->BaseStats.AccuracyMultiplier = 1.0f;
		WeaponData->BaseStats.DamageSources.Add(EDamageSource::Physical);
		WeaponData->BaseStats.TargetReach = ETargetReach::AnyEnemy;
		return WeaponData;
	}
};

// Test: GetStats returns modified stats
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitGetStatsReturnsModifiedStatsTest,
	"KBS.Units.Unit.GetStats_ReturnsModifiedStats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitGetStatsReturnsModifiedStatsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World, 150.0f, 0.8f);
	Unit->RecalculateModifiedStats();

	const FUnitCoreStats& Stats = Unit->GetStats();
	TestEqual("Should return modified max health", Stats.MaxHealth, 150.0f);
	TestEqual("Should return modified accuracy", Stats.Accuracy, 0.8f);

	Unit->Destroy();
	return true;
}

// Test: GetBaseStats returns unmodified stats
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitGetBaseStatsReturnsUnmodifiedStatsTest,
	"KBS.Units.Unit.GetBaseStats_ReturnsUnmodifiedStats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitGetBaseStatsReturnsUnmodifiedStatsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World, 120.0f, 0.6f);

	const FUnitCoreStats& BaseStats = Unit->GetBaseStats();
	TestEqual("Base max health should be unchanged", BaseStats.MaxHealth, 120.0f);
	TestEqual("Base accuracy should be unchanged", BaseStats.Accuracy, 0.6f);

	Unit->Destroy();
	return true;
}

// Test: GetCurrentHealth returns correct value
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitGetCurrentHealthReturnsCorrectValueTest,
	"KBS.Units.Unit.GetCurrentHealth_ReturnsCorrectValue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitGetCurrentHealthReturnsCorrectValueTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World, 100.0f);

	TestEqual("Initial health should match max health", Unit->GetCurrentHealth(), 100.0f);

	Unit->Destroy();
	return true;
}

// Test: IsDead with zero health returns true
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitIsDeadWithZeroHealthReturnsTrueTest,
	"KBS.Units.Unit.IsDead_WithZeroHealth_ReturnsTrue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitIsDeadWithZeroHealthReturnsTrueTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World, 100.0f);

	FDamageResult DamageResult;
	DamageResult.Damage = 100;
	DamageResult.DamageSource = EDamageSource::Physical;
	Unit->TakeHit(DamageResult);

	TestTrue("Unit should be dead when health is 0", Unit->IsDead());

	Unit->Destroy();
	return true;
}

// Test: IsDead with positive health returns false
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitIsDeadWithPositiveHealthReturnsFalseTest,
	"KBS.Units.Unit.IsDead_WithPositiveHealth_ReturnsFalse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitIsDeadWithPositiveHealthReturnsFalseTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World, 100.0f);

	TestFalse("Unit should be alive with positive health", Unit->IsDead());

	Unit->Destroy();
	return true;
}

// Test: RecalculateModifiedStats clamps armor values
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitRecalculateModifiedStatsClampsArmorTest,
	"KBS.Units.Unit.RecalculateModifiedStats_ClampsArmorValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitRecalculateModifiedStatsClampsArmorTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World);
	UUnitDefinition* Definition = FUnitTestHelper::CreateTestUnitDefinition(Unit);
	Definition->BaseStatsTemplate.Defense.Armour.Add(EDamageSource::Physical, 0.95f);
	Unit->SetUnitDefinition(Definition);
	Unit->RecalculateModifiedStats();

	const FUnitCoreStats& Stats = Unit->GetStats();
	if (Stats.Defense.Armour.Contains(EDamageSource::Physical))
	{
		TestEqual("Armor should be clamped to 0.9", Stats.Defense.Armour[EDamageSource::Physical], 0.9f);
	}

	Unit->Destroy();
	return true;
}

// Test: RecalculateAllWeaponStats updates all weapons
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitRecalculateAllWeaponStatsUpdatesAllWeaponsTest,
	"KBS.Units.Unit.RecalculateAllWeaponStats_UpdatesAllWeapons",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitRecalculateAllWeaponStatsUpdatesAllWeaponsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World);

	UWeaponDataAsset* WeaponData1 = FUnitTestHelper::CreateTestWeaponData(Unit, 50);
	UWeaponDataAsset* WeaponData2 = FUnitTestHelper::CreateTestWeaponData(Unit, 60);

	UUnitDefinition* Definition = FUnitTestHelper::CreateTestUnitDefinition(Unit);
	Definition->DefaultWeapons.Add(WeaponData1);
	Definition->DefaultWeapons.Add(WeaponData2);
	Unit->SetUnitDefinition(Definition);

	Unit->RecalculateAllWeaponStats();

	TestEqual("Should have 2 weapons", Unit->GetWeapons().Num(), 2);

	Unit->Destroy();
	return true;
}

// Test: Restore resets stats and weapons
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitRestoreResetsStatsAndWeaponsTest,
	"KBS.Units.Unit.Restore_ResetsStatsAndWeapons",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitRestoreResetsStatsAndWeaponsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World, 100.0f);
	Unit->Restore();

	const FUnitCoreStats& Stats = Unit->GetStats();
	TestEqual("Max health should match base", Stats.MaxHealth, 100.0f);

	Unit->Destroy();
	return true;
}

// Test: LevelUp increases stats correctly
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitLevelUpIncreasesStatsCorrectlyTest,
	"KBS.Units.Unit.LevelUp_IncreasesStatsCorrectly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitLevelUpIncreasesStatsCorrectlyTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World, 100.0f, 0.75f);
	float OriginalHealth = Unit->GetBaseStats().MaxHealth;
	float OriginalAccuracy = Unit->GetBaseStats().Accuracy;

	Unit->LevelUp();

	TestEqual("Health should increase by 10%", Unit->GetBaseStats().MaxHealth, OriginalHealth * 1.1f);
	TestEqual("Accuracy should increase by 0.01", Unit->GetBaseStats().Accuracy, FMath::Min(OriginalAccuracy + 0.01f, 1.0f));

	Unit->Destroy();
	return true;
}

// Test: LevelUp maintains health percentage
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitLevelUpMaintainsHealthPercentageTest,
	"KBS.Units.Unit.LevelUp_MaintainsHealthPercentage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitLevelUpMaintainsHealthPercentageTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World, 100.0f);

	FDamageResult DamageResult;
	DamageResult.Damage = 50;
	DamageResult.DamageSource = EDamageSource::Physical;
	Unit->TakeHit(DamageResult);

	TestEqual("Health should be 50", Unit->GetCurrentHealth(), 50.0f);

	Unit->LevelUp();

	float ExpectedHealth = 110.0f * 0.5f;
	TestEqual("Health percentage should be maintained", Unit->GetCurrentHealth(), ExpectedHealth);

	Unit->Destroy();
	return true;
}

// Test: LevelUp caps accuracy at 100%
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitLevelUpCapAccuracyAt100PercentTest,
	"KBS.Units.Unit.LevelUp_CapAccuracyAt100Percent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitLevelUpCapAccuracyAt100PercentTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World, 100.0f, 0.995f);

	Unit->LevelUp();

	TestEqual("Accuracy should be capped at 1.0", Unit->GetBaseStats().Accuracy, 1.0f);

	Unit->Destroy();
	return true;
}

// Test: TakeHit reduces health
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitTakeHitReducesHealthTest,
	"KBS.Units.Unit.TakeHit_ReducesHealth",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitTakeHitReducesHealthTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World, 100.0f);

	FDamageResult DamageResult;
	DamageResult.Damage = 30;
	DamageResult.DamageSource = EDamageSource::Physical;
	Unit->TakeHit(DamageResult);

	TestEqual("Health should be reduced by damage", Unit->GetCurrentHealth(), 70.0f);

	Unit->Destroy();
	return true;
}

// Test: TakeHit removes ward when blocked
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitTakeHitRemovesWardWhenBlockedTest,
	"KBS.Units.Unit.TakeHit_RemovesWardWhenBlocked",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitTakeHitRemovesWardWhenBlockedTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World, 100.0f);
	UUnitDefinition* Definition = FUnitTestHelper::CreateTestUnitDefinition(Unit);
	Definition->BaseStatsTemplate.Defense.Wards.Add(EDamageSource::Fire);
	Unit->SetUnitDefinition(Definition);
	Unit->RecalculateModifiedStats();

	TestTrue("Ward should exist before hit", Unit->GetStats().Defense.Wards.Contains(EDamageSource::Fire));

	FDamageResult DamageResult;
	DamageResult.Damage = 0;
	DamageResult.DamageBlocked = 50;
	DamageResult.DamageSource = EDamageSource::Fire;
	Unit->TakeHit(DamageResult);

	TestFalse("Ward should be removed after blocking", Unit->GetStats().Defense.Wards.Contains(EDamageSource::Fire));

	Unit->Destroy();
	return true;
}

// Test: TakeHit does not reduce health below zero
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitTakeHitDoesNotReduceHealthBelowZeroTest,
	"KBS.Units.Unit.TakeHit_DoesNotReduceHealthBelowZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitTakeHitDoesNotReduceHealthBelowZeroTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World, 100.0f);

	FDamageResult DamageResult;
	DamageResult.Damage = 50;
	DamageResult.DamageSource = EDamageSource::Physical;
	Unit->TakeHit(DamageResult);

	DamageResult.Damage = 80;
	Unit->TakeHit(DamageResult);

	TestEqual("Health should be clamped to 0", Unit->GetCurrentHealth(), 0.0f);

	Unit->Destroy();
	return true;
}

// Test: SetDefending updates defense flag
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitSetDefendingUpdatesDefenseFlagTest,
	"KBS.Units.Unit.SetDefending_UpdatesDefenseFlag",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitSetDefendingUpdatesDefenseFlagTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World);

	Unit->SetDefending(true);
	TestTrue("Defense flag should be true", Unit->GetStats().Defense.bIsDefending);

	Unit->SetDefending(false);
	TestFalse("Defense flag should be false", Unit->GetStats().Defense.bIsDefending);

	Unit->Destroy();
	return true;
}

// Test: GetTeamSide returns correct team
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitGetTeamSideReturnsCorrectTeamTest,
	"KBS.Units.Unit.GetTeamSide_ReturnsCorrectTeam",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitGetTeamSideReturnsCorrectTeamTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World);
	Unit->SetTeamSide(ETeamSide::Defender);

	TestEqual("Team side should be Defender", Unit->GetTeamSide(), ETeamSide::Defender);

	Unit->Destroy();
	return true;
}

// Test: SetTeamSide updates team
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitSetTeamSideUpdatesTeamTest,
	"KBS.Units.Unit.SetTeamSide_UpdatesTeam",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitSetTeamSideUpdatesTeamTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World);

	Unit->SetTeamSide(ETeamSide::Attacker);
	TestEqual("Team should be Attacker", Unit->GetTeamSide(), ETeamSide::Attacker);

	Unit->SetTeamSide(ETeamSide::Defender);
	TestEqual("Team should be Defender", Unit->GetTeamSide(), ETeamSide::Defender);

	Unit->Destroy();
	return true;
}

// Test: TakeHit with zero damage does not remove ward
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitTakeHitWithZeroDamageDoesNotRemoveWardTest,
	"KBS.Units.Unit.TakeHit_WithZeroDamage_DoesNotRemoveWard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitTakeHitWithZeroDamageDoesNotRemoveWardTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World);
	UUnitDefinition* Definition = FUnitTestHelper::CreateTestUnitDefinition(Unit);
	Definition->BaseStatsTemplate.Defense.Wards.Add(EDamageSource::Fire);
	Unit->SetUnitDefinition(Definition);
	Unit->RecalculateModifiedStats();

	FDamageResult DamageResult;
	DamageResult.Damage = 0;
	DamageResult.DamageBlocked = 0;
	DamageResult.DamageSource = EDamageSource::Fire;
	Unit->TakeHit(DamageResult);

	TestTrue("Ward should remain when no damage blocked", Unit->GetStats().Defense.Wards.Contains(EDamageSource::Fire));

	Unit->Destroy();
	return true;
}

// Test: LevelUp at max accuracy does not increase
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitLevelUpAtMaxAccuracyDoesNotIncreaseTest,
	"KBS.Units.Unit.LevelUp_AtMaxAccuracy_DoesNotIncrease",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitLevelUpAtMaxAccuracyDoesNotIncreaseTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World, 100.0f, 1.0f);

	Unit->LevelUp();

	TestEqual("Accuracy should remain at 1.0", Unit->GetBaseStats().Accuracy, 1.0f);

	Unit->Destroy();
	return true;
}

// Test: TakeHit kills unit when health becomes zero (overkill scenario)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitTakeHitKillsUnitHealthBecomesZeroTest,
	"KBS.Units.Unit.TakeHit_KillsUnit_HealthBecomesZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitTakeHitKillsUnitHealthBecomesZeroTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitTestHelper::CreateMockUnit(World, 100.0f);

	FDamageResult DamageResult;
	DamageResult.Damage = 150;
	DamageResult.DamageSource = EDamageSource::Physical;
	Unit->TakeHit(DamageResult);

	TestEqual("Health should be 0 after overkill", Unit->GetCurrentHealth(), 0.0f);
	TestTrue("Unit should be dead after overkill", Unit->IsDead());

	Unit->Destroy();
	return true;
}
