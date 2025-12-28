#include "Misc/AutomationTest.h"
#include "GameMechanics/Units/Abilities/AbilityFactory.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Abilities/UnitWaitAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitDefinition.h"

class FAbilityFactoryTestHelper
{
public:
	static UUnitDefinition* CreateTestUnitDefinition(UObject* Outer)
	{
		UUnitDefinition* Definition = NewObject<UUnitDefinition>(Outer);
		Definition->UnitName = TEXT("TestUnit");
		Definition->BaseStatsTemplate.MaxHealth = 100.0f;
		return Definition;
	}

	static AUnit* CreateMockUnit(UWorld* World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AUnit* Unit = World->SpawnActor<AUnit>(AUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (Unit)
		{
			UUnitDefinition* Definition = CreateTestUnitDefinition(Unit);
			Unit->SetUnitDefinition(Definition);
		}
		return Unit;
	}

	static UUnitAbilityDefinition* CreateTestAbilityDefinition(UObject* Outer, TSubclassOf<UUnitAbilityInstance> AbilityClass = UUnitWaitAbility::StaticClass())
	{
		UUnitAbilityDefinition* Definition = NewObject<UUnitAbilityDefinition>(Outer);
		Definition->AbilityClass = AbilityClass;
		Definition->AbilityName = TEXT("TestAbility");
		Definition->MaxCharges = 3;
		Definition->Targeting = ETargetReach::AnyEnemy;
		return Definition;
	}
};

// Test: CreateAbilityFromDefinition creates ability successfully
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityFactoryCreateAbilityValidTest,
	"KBS.Units.Abilities.AbilityFactory.CreateAbilityFromDefinition_Valid_CreatesAbility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityFactoryCreateAbilityValidTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityFactoryTestHelper::CreateMockUnit(World);
	UUnitAbilityDefinition* Definition = FAbilityFactoryTestHelper::CreateTestAbilityDefinition(Unit);

	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);

	TestNotNull("Ability should be created", Ability);

	Unit->Destroy();
	return true;
}

// Test: CreateAbilityFromDefinition with null definition returns null
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityFactoryCreateAbilityNullDefinitionTest,
	"KBS.Units.Abilities.AbilityFactory.CreateAbilityFromDefinition_NullDefinition_ReturnsNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityFactoryCreateAbilityNullDefinitionTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityFactoryTestHelper::CreateMockUnit(World);

	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(nullptr, Unit);

	TestNull("Ability should be null when definition is null", Ability);

	Unit->Destroy();
	return true;
}

// Test: CreateAbilityFromDefinition with null owner returns null
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityFactoryCreateAbilityNullOwnerTest,
	"KBS.Units.Abilities.AbilityFactory.CreateAbilityFromDefinition_NullOwner_ReturnsNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityFactoryCreateAbilityNullOwnerTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityFactoryTestHelper::CreateMockUnit(World);
	UUnitAbilityDefinition* Definition = FAbilityFactoryTestHelper::CreateTestAbilityDefinition(Unit);

	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, nullptr);

	TestNull("Ability should be null when owner is null", Ability);

	Unit->Destroy();
	return true;
}

// Test: CreateAbilityFromDefinition with null ability class returns null
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityFactoryCreateAbilityNullClassTest,
	"KBS.Units.Abilities.AbilityFactory.CreateAbilityFromDefinition_NullAbilityClass_ReturnsNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityFactoryCreateAbilityNullClassTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityFactoryTestHelper::CreateMockUnit(World);
	UUnitAbilityDefinition* Definition = FAbilityFactoryTestHelper::CreateTestAbilityDefinition(Unit);
	Definition->AbilityClass = nullptr;

	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);

	TestNull("Ability should be null when AbilityClass is null", Ability);

	Unit->Destroy();
	return true;
}

// Test: CreateAbilityFromDefinition initializes ability correctly
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAbilityFactoryCreateAbilityInitializesCorrectlyTest,
	"KBS.Units.Abilities.AbilityFactory.CreateAbilityFromDefinition_InitializesCorrectly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FAbilityFactoryCreateAbilityInitializesCorrectlyTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FAbilityFactoryTestHelper::CreateMockUnit(World);
	UUnitAbilityDefinition* Definition = FAbilityFactoryTestHelper::CreateTestAbilityDefinition(Unit);
	Definition->MaxCharges = 7;

	UUnitAbilityInstance* Ability = UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit);

	TestNotNull("Ability should be created", Ability);
	TestEqual("Config should be set to definition", Ability->GetConfig(), Definition);
	TestEqual("Owner should be set to unit", Ability->GetOwner(), Unit);
	TestEqual("RemainingCharges should match MaxCharges", Ability->GetRemainingCharges(), 7);

	Unit->Destroy();
	return true;
}
