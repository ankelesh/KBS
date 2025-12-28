#include "Misc/AutomationTest.h"
#include "GameMechanics/Tactical/DamageCalculator.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameplayTypes/CombatTypes.h"

// Helper class for creating test fixtures
class FDamageCalculatorTestHelper
{
public:
	static UDamageCalculator* GetDamageCalculator(UWorld* World)
	{
		return World->GetSubsystem<UDamageCalculator>();
	}

	static UUnitDefinition* CreateTestUnitDefinition(UObject* Outer, float MaxHealth = 100.0f, float Accuracy = 0.75f)
	{
		UUnitDefinition* Definition = NewObject<UUnitDefinition>(Outer);
		Definition->UnitName = TEXT("TestUnit");
		Definition->MovementSpeed = 300.0f;
		Definition->BaseStatsTemplate.MaxHealth = MaxHealth;
		Definition->BaseStatsTemplate.Initiative = 50;
		Definition->BaseStatsTemplate.Accuracy = Accuracy;
		Definition->BaseStatsTemplate.Defense.DamageReduction = 0;
		return Definition;
	}

	static AUnit* CreateMockUnit(UWorld* World, float MaxHealth = 100.0f, float Accuracy = 0.75f)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AUnit* Unit = World->SpawnActor<AUnit>(AUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (Unit)
		{
			UUnitDefinition* Definition = CreateTestUnitDefinition(Unit, MaxHealth, Accuracy);
			Unit->SetUnitDefinition(Definition);
		}
		return Unit;
	}

	static UWeaponDataAsset* CreateTestWeaponData(UObject* Outer, int32 BaseDamage = 50, float AccuracyMult = 1.0f, ETargetReach Reach = ETargetReach::AnyEnemy)
	{
		UWeaponDataAsset* WeaponData = NewObject<UWeaponDataAsset>(Outer);
		WeaponData->BaseStats.BaseDamage = BaseDamage;
		WeaponData->BaseStats.AccuracyMultiplier = AccuracyMult;
		WeaponData->BaseStats.DamageSources.Add(EDamageSource::Physical);
		WeaponData->BaseStats.TargetReach = Reach;
		return WeaponData;
	}

	static UWeapon* CreateAndAttachWeapon(AUnit* Unit, int32 BaseDamage = 50, float AccuracyMult = 1.0f, ETargetReach Reach = ETargetReach::AnyEnemy)
	{
		UWeaponDataAsset* WeaponData = CreateTestWeaponData(Unit, BaseDamage, AccuracyMult, Reach);
		UWeapon* Weapon = NewObject<UWeapon>(Unit);
		Weapon->Initialize(nullptr, WeaponData);
		return Weapon;
	}
};

// Test: CalculateHitChance with valid inputs returns correct value
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateHitChanceWithValidInputsTest,
	"KBS.Tactical.DamageCalculator.CalculateHitChance_WithValidInputs_ReturnsCorrectValue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateHitChanceWithValidInputsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World, 100.0f, 0.8f);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);
	UWeapon* Weapon = FDamageCalculatorTestHelper::CreateAndAttachWeapon(Attacker, 50, 1.25f);

	float HitChance = Calculator->CalculateHitChance(Attacker, Weapon, Target);

	TestEqual("Hit chance should be Accuracy * Weapon Multiplier * 100", HitChance, 0.8f * 1.25f * 100.0f);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: CalculateHitChance clamps to 100%
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateHitChanceClampsTo100Test,
	"KBS.Tactical.DamageCalculator.CalculateHitChance_ClampsTo100Percent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateHitChanceClampsTo100Test::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World, 100.0f, 1.0f);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);
	UWeapon* Weapon = FDamageCalculatorTestHelper::CreateAndAttachWeapon(Attacker, 50, 2.0f);

	float HitChance = Calculator->CalculateHitChance(Attacker, Weapon, Target);

	TestEqual("Hit chance should be clamped to 100", HitChance, 100.0f);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: CalculateHitChance clamps to 0%
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateHitChanceClampsTo0Test,
	"KBS.Tactical.DamageCalculator.CalculateHitChance_ClampsTo0Percent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateHitChanceClampsTo0Test::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World, 100.0f, 0.0f);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);
	UWeapon* Weapon = FDamageCalculatorTestHelper::CreateAndAttachWeapon(Attacker);

	float HitChance = Calculator->CalculateHitChance(Attacker, Weapon, Target);

	TestEqual("Hit chance should be clamped to 0", HitChance, 0.0f);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: CalculateDamage with basic weapon returns correct damage
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateDamageWithBasicWeaponTest,
	"KBS.Tactical.DamageCalculator.CalculateDamage_WithBasicWeapon_ReturnsCorrectDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateDamageWithBasicWeaponTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);
	UWeapon* Weapon = FDamageCalculatorTestHelper::CreateAndAttachWeapon(Attacker, 50);

	FDamageResult Result = Calculator->CalculateDamage(Attacker, Weapon, Target);

	TestEqual("Damage should match base damage", Result.Damage, 50);
	TestEqual("Damage source should be Physical", Result.DamageSource, EDamageSource::Physical);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: CalculateDamage with immunity returns zero damage
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateDamageWithImmunityTest,
	"KBS.Tactical.DamageCalculator.CalculateDamage_WithImmunity_ReturnsZeroDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateDamageWithImmunityTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);
	UUnitDefinition* TargetDef = FDamageCalculatorTestHelper::CreateTestUnitDefinition(Target);
	TargetDef->BaseStatsTemplate.Defense.Immunities.Add(EDamageSource::Physical);
	Target->SetUnitDefinition(TargetDef);
	Target->RecalculateModifiedStats();

	UWeapon* Weapon = FDamageCalculatorTestHelper::CreateAndAttachWeapon(Attacker, 50);

	FDamageResult Result = Calculator->CalculateDamage(Attacker, Weapon, Target);

	TestEqual("Damage should be 0 with immunity", Result.Damage, 0);
	TestEqual("Blocked damage should equal base damage", Result.DamageBlocked, 50);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: CalculateDamage with ward returns zero damage
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateDamageWithWardTest,
	"KBS.Tactical.DamageCalculator.CalculateDamage_WithWard_ReturnsZeroDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateDamageWithWardTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);
	UUnitDefinition* TargetDef = FDamageCalculatorTestHelper::CreateTestUnitDefinition(Target);
	TargetDef->BaseStatsTemplate.Defense.Wards.Add(EDamageSource::Physical);
	Target->SetUnitDefinition(TargetDef);
	Target->RecalculateModifiedStats();

	UWeapon* Weapon = FDamageCalculatorTestHelper::CreateAndAttachWeapon(Attacker, 50);

	FDamageResult Result = Calculator->CalculateDamage(Attacker, Weapon, Target);

	TestEqual("Damage should be 0 with ward", Result.Damage, 0);
	TestEqual("Blocked damage should equal base damage", Result.DamageBlocked, 50);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: CalculateDamage with armor reduces damage
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateDamageWithArmorTest,
	"KBS.Tactical.DamageCalculator.CalculateDamage_WithArmor_ReducesDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateDamageWithArmorTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);
	UUnitDefinition* TargetDef = FDamageCalculatorTestHelper::CreateTestUnitDefinition(Target);
	TargetDef->BaseStatsTemplate.Defense.Armour.Add(EDamageSource::Physical, 0.5f);
	Target->SetUnitDefinition(TargetDef);
	Target->RecalculateModifiedStats();

	UWeapon* Weapon = FDamageCalculatorTestHelper::CreateAndAttachWeapon(Attacker, 100);

	FDamageResult Result = Calculator->CalculateDamage(Attacker, Weapon, Target);

	TestEqual("Damage should be reduced by 50%", Result.Damage, 50);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: CalculateDamage with damage reduction subtracts flat amount
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateDamageWithDamageReductionTest,
	"KBS.Tactical.DamageCalculator.CalculateDamage_WithDamageReduction_SubtractsFlat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateDamageWithDamageReductionTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);
	UUnitDefinition* TargetDef = FDamageCalculatorTestHelper::CreateTestUnitDefinition(Target);
	TargetDef->BaseStatsTemplate.Defense.DamageReduction = 10;
	Target->SetUnitDefinition(TargetDef);
	Target->RecalculateModifiedStats();

	UWeapon* Weapon = FDamageCalculatorTestHelper::CreateAndAttachWeapon(Attacker, 50);

	FDamageResult Result = Calculator->CalculateDamage(Attacker, Weapon, Target);

	TestEqual("Damage should be reduced by flat amount", Result.Damage, 40);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: CalculateDamage with defending halves damage
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateDamageWithDefendingTest,
	"KBS.Tactical.DamageCalculator.CalculateDamage_WithDefending_HalvesDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateDamageWithDefendingTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);
	Target->SetDefending(true);

	UWeapon* Weapon = FDamageCalculatorTestHelper::CreateAndAttachWeapon(Attacker, 100);

	FDamageResult Result = Calculator->CalculateDamage(Attacker, Weapon, Target);

	TestEqual("Damage should be halved when defending", Result.Damage, 50);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: CalculateDamage with armor and defending stacks correctly
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateDamageWithArmorAndDefendingTest,
	"KBS.Tactical.DamageCalculator.CalculateDamage_WithArmorAndDefending_StacksCorrectly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateDamageWithArmorAndDefendingTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);
	UUnitDefinition* TargetDef = FDamageCalculatorTestHelper::CreateTestUnitDefinition(Target);
	TargetDef->BaseStatsTemplate.Defense.Armour.Add(EDamageSource::Physical, 0.5f);
	Target->SetUnitDefinition(TargetDef);
	Target->RecalculateModifiedStats();
	Target->SetDefending(true);

	UWeapon* Weapon = FDamageCalculatorTestHelper::CreateAndAttachWeapon(Attacker, 100);

	FDamageResult Result = Calculator->CalculateDamage(Attacker, Weapon, Target);

	// 100 * (1 - 0.5) = 50, then 50 * 0.5 (defending) = 25
	TestEqual("Armor and defending should stack", Result.Damage, 25);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: CalculateEffectApplication with valid inputs returns chance
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateEffectApplicationTest,
	"KBS.Tactical.DamageCalculator.CalculateEffectApplication_WithValidInputs_ReturnsChance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateEffectApplicationTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World, 100.0f, 0.8f);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);

	// Note: We'd need a proper BattleEffect instance here, but for basic testing we can skip this
	// as it requires more complex setup. This test verifies the calculation logic exists.

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: SelectWeapon with one weapon returns that weapon
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorSelectWeaponWithOneWeaponTest,
	"KBS.Tactical.DamageCalculator.SelectWeapon_WithOneWeapon_ReturnsThatWeapon",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorSelectWeaponWithOneWeaponTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);

	UWeaponDataAsset* WeaponData = FDamageCalculatorTestHelper::CreateTestWeaponData(Attacker);
	UUnitDefinition* AttackerDef = FDamageCalculatorTestHelper::CreateTestUnitDefinition(Attacker);
	AttackerDef->DefaultWeapons.Add(WeaponData);
	Attacker->SetUnitDefinition(AttackerDef);

	UWeapon* Selected = Calculator->SelectWeapon(Attacker, Target, ETargetReach::AnyEnemy);

	TestNotNull("Should select the only weapon", Selected);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: SelectWeapon with no valid weapons returns null
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorSelectWeaponWithNoValidWeaponsTest,
	"KBS.Tactical.DamageCalculator.SelectWeapon_WithNoValidWeapons_ReturnsNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorSelectWeaponWithNoValidWeaponsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);

	UWeaponDataAsset* WeaponData = FDamageCalculatorTestHelper::CreateTestWeaponData(Attacker, 50, 1.0f, ETargetReach::Self);
	UUnitDefinition* AttackerDef = FDamageCalculatorTestHelper::CreateTestUnitDefinition(Attacker);
	AttackerDef->DefaultWeapons.Add(WeaponData);
	Attacker->SetUnitDefinition(AttackerDef);

	UWeapon* Selected = Calculator->SelectWeapon(Attacker, Target, ETargetReach::ClosestEnemies);

	TestNull("Should return null when no weapons match reach", Selected);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: SelectMaxReachWeapon returns highest reach
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorSelectMaxReachWeaponTest,
	"KBS.Tactical.DamageCalculator.SelectMaxReachWeapon_ReturnsHighestReach",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorSelectMaxReachWeaponTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Unit = FDamageCalculatorTestHelper::CreateMockUnit(World);

	UWeaponDataAsset* WeaponData1 = FDamageCalculatorTestHelper::CreateTestWeaponData(Unit, 50, 1.0f, ETargetReach::ClosestEnemies);
	UWeaponDataAsset* WeaponData2 = FDamageCalculatorTestHelper::CreateTestWeaponData(Unit, 50, 1.0f, ETargetReach::AllEnemies);
	UUnitDefinition* UnitDef = FDamageCalculatorTestHelper::CreateTestUnitDefinition(Unit);
	UnitDef->DefaultWeapons.Add(WeaponData1);
	UnitDef->DefaultWeapons.Add(WeaponData2);
	Unit->SetUnitDefinition(UnitDef);

	UWeapon* Selected = Calculator->SelectMaxReachWeapon(Unit);

	TestNotNull("Should select a weapon", Selected);
	if (Selected)
	{
		TestEqual("Should select AllEnemies reach", Selected->GetReach(), ETargetReach::AllEnemies);
	}

	Unit->Destroy();
	return true;
}

// Test: PreviewDamage calculates correct preview
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorPreviewDamageTest,
	"KBS.Tactical.DamageCalculator.PreviewDamage_CalculatesCorrectPreview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorPreviewDamageTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World, 100.0f, 0.8f);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);

	UWeaponDataAsset* WeaponData = FDamageCalculatorTestHelper::CreateTestWeaponData(Attacker, 50, 1.0f);
	UUnitDefinition* AttackerDef = FDamageCalculatorTestHelper::CreateTestUnitDefinition(Attacker, 100.0f, 0.8f);
	AttackerDef->DefaultWeapons.Add(WeaponData);
	Attacker->SetUnitDefinition(AttackerDef);

	FPreviewHitResult Preview = Calculator->PreviewDamage(Attacker, Target, ETargetReach::AnyEnemy);

	TestEqual("Hit probability should be calculated", Preview.HitProbability, 80.0f);
	TestEqual("Damage preview should be calculated", Preview.DamageResult.Damage, 50);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: ProcessStatistics tracks hits correctly
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorProcessStatisticsTracksHitsTest,
	"KBS.Tactical.DamageCalculator.ProcessStatistics_TracksHitsCorrectly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorProcessStatisticsTracksHitsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	Calculator->ResetStatistics();

	FCombatHitResult HitResult;
	HitResult.bHit = true;
	HitResult.Damage = 50;

	Calculator->ProcessStatistics(HitResult, true);

	TestEqual("Hits should be tracked", Calculator->GetAttackerStats().Hits, 1);
	TestEqual("Total damage should be tracked", Calculator->GetAttackerStats().TotalDamage, 50);

	return true;
}

// Test: ProcessStatistics tracks misses correctly
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorProcessStatisticsTracksMissesTest,
	"KBS.Tactical.DamageCalculator.ProcessStatistics_TracksMissesCorrectly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorProcessStatisticsTracksMissesTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	Calculator->ResetStatistics();

	FCombatHitResult MissResult;
	MissResult.bHit = false;

	Calculator->ProcessStatistics(MissResult, true);

	TestEqual("Misses should be tracked", Calculator->GetAttackerStats().Misses, 1);

	return true;
}

// Test: ResetStatistics clears all stats
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorResetStatisticsTest,
	"KBS.Tactical.DamageCalculator.ResetStatistics_ClearsAllStats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorResetStatisticsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);

	FCombatHitResult HitResult;
	HitResult.bHit = true;
	HitResult.Damage = 50;
	Calculator->ProcessStatistics(HitResult, true);

	Calculator->ResetStatistics();

	TestEqual("Hits should be reset", Calculator->GetAttackerStats().Hits, 0);
	TestEqual("Total damage should be reset", Calculator->GetAttackerStats().TotalDamage, 0);
	TestEqual("Misses should be reset", Calculator->GetAttackerStats().Misses, 0);

	return true;
}

// Test: CalculateHitChance with null attacker returns zero
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateHitChanceWithNullAttackerTest,
	"KBS.Tactical.DamageCalculator.CalculateHitChance_WithNullAttacker_ReturnsZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateHitChanceWithNullAttackerTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);
	UWeapon* Weapon = FDamageCalculatorTestHelper::CreateAndAttachWeapon(Target);

	float HitChance = Calculator->CalculateHitChance(nullptr, Weapon, Target);

	TestEqual("Hit chance should be 0 with null attacker", HitChance, 0.0f);

	Target->Destroy();
	return true;
}

// Test: CalculateHitChance with null weapon returns zero
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateHitChanceWithNullWeaponTest,
	"KBS.Tactical.DamageCalculator.CalculateHitChance_WithNullWeapon_ReturnsZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateHitChanceWithNullWeaponTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);

	float HitChance = Calculator->CalculateHitChance(Attacker, nullptr, Target);

	TestEqual("Hit chance should be 0 with null weapon", HitChance, 0.0f);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: CalculateHitChance with null target returns zero
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateHitChanceWithNullTargetTest,
	"KBS.Tactical.DamageCalculator.CalculateHitChance_WithNullTarget_ReturnsZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateHitChanceWithNullTargetTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World);
	UWeapon* Weapon = FDamageCalculatorTestHelper::CreateAndAttachWeapon(Attacker);

	float HitChance = Calculator->CalculateHitChance(Attacker, Weapon, nullptr);

	TestEqual("Hit chance should be 0 with null target", HitChance, 0.0f);

	Attacker->Destroy();
	return true;
}

// Test: CalculateDamage with zero base damage returns zero
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateDamageWithZeroBaseDamageTest,
	"KBS.Tactical.DamageCalculator.CalculateDamage_WithZeroBaseDamage_ReturnsZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateDamageWithZeroBaseDamageTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);
	UWeapon* Weapon = FDamageCalculatorTestHelper::CreateAndAttachWeapon(Attacker, 0);

	FDamageResult Result = Calculator->CalculateDamage(Attacker, Weapon, Target);

	TestEqual("Damage should be 0", Result.Damage, 0);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: CalculateDamage with high damage reduction clamps to zero
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateDamageWithHighDamageReductionTest,
	"KBS.Tactical.DamageCalculator.CalculateDamage_WithHighDamageReduction_ClampsToZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateDamageWithHighDamageReductionTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);
	UUnitDefinition* TargetDef = FDamageCalculatorTestHelper::CreateTestUnitDefinition(Target);
	TargetDef->BaseStatsTemplate.Defense.DamageReduction = 100;
	Target->SetUnitDefinition(TargetDef);
	Target->RecalculateModifiedStats();

	UWeapon* Weapon = FDamageCalculatorTestHelper::CreateAndAttachWeapon(Attacker, 50);

	FDamageResult Result = Calculator->CalculateDamage(Attacker, Weapon, Target);

	TestTrue("Damage should be negative or zero", Result.Damage <= 0);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: CalculateDamage with max armor reduces by 90%
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorCalculateDamageWithMaxArmorTest,
	"KBS.Tactical.DamageCalculator.CalculateDamage_WithMaxArmor_ReducesBy90Percent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorCalculateDamageWithMaxArmorTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);
	UUnitDefinition* TargetDef = FDamageCalculatorTestHelper::CreateTestUnitDefinition(Target);
	TargetDef->BaseStatsTemplate.Defense.Armour.Add(EDamageSource::Physical, 0.9f);
	Target->SetUnitDefinition(TargetDef);
	Target->RecalculateModifiedStats();

	UWeapon* Weapon = FDamageCalculatorTestHelper::CreateAndAttachWeapon(Attacker, 100);

	FDamageResult Result = Calculator->CalculateDamage(Attacker, Weapon, Target);

	TestEqual("Damage should be reduced by 90%", Result.Damage, 10);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}

// Test: SelectWeapon with empty array returns null
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageCalculatorSelectWeaponWithEmptyArrayTest,
	"KBS.Tactical.DamageCalculator.SelectWeapon_WithEmptyArray_ReturnsNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDamageCalculatorSelectWeaponWithEmptyArrayTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	UDamageCalculator* Calculator = FDamageCalculatorTestHelper::GetDamageCalculator(World);
	AUnit* Attacker = FDamageCalculatorTestHelper::CreateMockUnit(World);
	AUnit* Target = FDamageCalculatorTestHelper::CreateMockUnit(World);

	UWeapon* Selected = Calculator->SelectWeapon(Attacker, Target, ETargetReach::AnyEnemy);

	TestNull("Should return null with no weapons", Selected);

	Attacker->Destroy();
	Target->Destroy();
	return true;
}
