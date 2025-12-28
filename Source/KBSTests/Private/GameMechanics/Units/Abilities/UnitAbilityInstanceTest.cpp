#include "Misc/AutomationTest.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Abilities/UnitWaitAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameplayTypes/AbilityBattleContext.h"
#include "GameplayTypes/AbilityTypes.h"
#include "GameplayTypes/DamageTypes.h"

class FUnitAbilityTestHelper
{
public:
	static UUnitDefinition* CreateTestUnitDefinition(UObject* Outer)
	{
		UUnitDefinition* Definition = NewObject<UUnitDefinition>(Outer);
		Definition->UnitName = TEXT("TestUnit");
		Definition->BaseStatsTemplate.MaxHealth = 100.0f;
		Definition->BaseStatsTemplate.Initiative = 50;
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
		Definition->bIsPassive = false;
		return Definition;
	}
};

// Test: InitializeFromDefinition sets config, owner, and charges correctly
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitAbilityInstanceInitializeFromDefinitionTest,
	"KBS.Units.Abilities.UnitAbilityInstance.InitializeFromDefinition_SetsAllProperties",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitAbilityInstanceInitializeFromDefinitionTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitAbilityTestHelper::CreateMockUnit(World);
	UUnitAbilityDefinition* Definition = FUnitAbilityTestHelper::CreateTestAbilityDefinition(Unit);

	UUnitAbilityInstance* Ability = NewObject<UUnitWaitAbility>(Unit);
	Ability->InitializeFromDefinition(Definition, Unit);

	TestEqual("Config should be set", Ability->GetConfig(), Definition);
	TestEqual("Owner should be set", Ability->GetOwner(), Unit);
	TestEqual("RemainingCharges should match MaxCharges", Ability->GetRemainingCharges(), 3);

	Unit->Destroy();
	return true;
}

// Test: GetTargeting returns targeting from config
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitAbilityInstanceGetTargetingTest,
	"KBS.Units.Abilities.UnitAbilityInstance.GetTargeting_ReturnsConfigTargeting",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitAbilityInstanceGetTargetingTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitAbilityTestHelper::CreateMockUnit(World);
	UUnitAbilityDefinition* Definition = FUnitAbilityTestHelper::CreateTestAbilityDefinition(Unit);
	Definition->Targeting = ETargetReach::ClosestEnemies;

	UUnitAbilityInstance* Ability = NewObject<UUnitWaitAbility>(Unit);
	Ability->InitializeFromDefinition(Definition, Unit);

	TestEqual("Should return config targeting", Ability->GetTargeting(), ETargetReach::ClosestEnemies);

	Unit->Destroy();
	return true;
}

// Test: IsPassive returns passive flag from config
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitAbilityInstanceIsPassiveTest,
	"KBS.Units.Abilities.UnitAbilityInstance.IsPassive_ReturnsConfigPassiveFlag",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitAbilityInstanceIsPassiveTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitAbilityTestHelper::CreateMockUnit(World);
	UUnitAbilityDefinition* Definition = FUnitAbilityTestHelper::CreateTestAbilityDefinition(Unit);
	Definition->bIsPassive = true;

	UUnitAbilityInstance* Ability = NewObject<UUnitWaitAbility>(Unit);
	Ability->InitializeFromDefinition(Definition, Unit);

	TestTrue("Should return true for passive ability", Ability->IsPassive());

	Definition->bIsPassive = false;
	Ability->InitializeFromDefinition(Definition, Unit);
	TestFalse("Should return false for active ability", Ability->IsPassive());

	Unit->Destroy();
	return true;
}

// Test: ConsumeCharge decrements remaining charges
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitAbilityInstanceConsumeChargeTest,
	"KBS.Units.Abilities.UnitAbilityInstance.ConsumeCharge_DecrementsCharges",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitAbilityInstanceConsumeChargeTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitAbilityTestHelper::CreateMockUnit(World);
	UUnitAbilityDefinition* Definition = FUnitAbilityTestHelper::CreateTestAbilityDefinition(Unit);
	Definition->MaxCharges = 5;

	UUnitAbilityInstance* Ability = NewObject<UUnitWaitAbility>(Unit);
	Ability->InitializeFromDefinition(Definition, Unit);

	TestEqual("Initial charges should be 5", Ability->GetRemainingCharges(), 5);

	Ability->ConsumeCharge();
	TestEqual("Charges should be 4 after consume", Ability->GetRemainingCharges(), 4);

	Ability->ConsumeCharge();
	TestEqual("Charges should be 3 after second consume", Ability->GetRemainingCharges(), 3);

	Unit->Destroy();
	return true;
}

// Test: ConsumeCharge at zero does not go below zero
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitAbilityInstanceConsumeChargeAtZeroTest,
	"KBS.Units.Abilities.UnitAbilityInstance.ConsumeCharge_AtZero_DoesNotGoBelowZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitAbilityInstanceConsumeChargeAtZeroTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitAbilityTestHelper::CreateMockUnit(World);
	UUnitAbilityDefinition* Definition = FUnitAbilityTestHelper::CreateTestAbilityDefinition(Unit);
	Definition->MaxCharges = 1;

	UUnitAbilityInstance* Ability = NewObject<UUnitWaitAbility>(Unit);
	Ability->InitializeFromDefinition(Definition, Unit);

	Ability->ConsumeCharge();
	TestEqual("Charges should be 0", Ability->GetRemainingCharges(), 0);

	Ability->ConsumeCharge();
	TestEqual("Charges should not go below 0", Ability->GetRemainingCharges(), 0);

	Unit->Destroy();
	return true;
}

// Test: RestoreCharges resets to max charges
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitAbilityInstanceRestoreChargesTest,
	"KBS.Units.Abilities.UnitAbilityInstance.RestoreCharges_ResetsToMaxCharges",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitAbilityInstanceRestoreChargesTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitAbilityTestHelper::CreateMockUnit(World);
	UUnitAbilityDefinition* Definition = FUnitAbilityTestHelper::CreateTestAbilityDefinition(Unit);
	Definition->MaxCharges = 4;

	UUnitAbilityInstance* Ability = NewObject<UUnitWaitAbility>(Unit);
	Ability->InitializeFromDefinition(Definition, Unit);

	Ability->ConsumeCharge();
	Ability->ConsumeCharge();
	TestEqual("Charges should be 2", Ability->GetRemainingCharges(), 2);

	Ability->RestoreCharges();
	TestEqual("Charges should be restored to 4", Ability->GetRemainingCharges(), 4);

	Unit->Destroy();
	return true;
}

// Test: CanExecute returns failure when charges depleted
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitAbilityInstanceCanExecuteNoChargesTest,
	"KBS.Units.Abilities.UnitAbilityInstance.CanExecute_NoCharges_ReturnsFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitAbilityInstanceCanExecuteNoChargesTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitAbilityTestHelper::CreateMockUnit(World);
	AUnit* Target = FUnitAbilityTestHelper::CreateMockUnit(World);
	UUnitAbilityDefinition* Definition = FUnitAbilityTestHelper::CreateTestAbilityDefinition(Unit);
	Definition->MaxCharges = 1;

	UUnitAbilityInstance* Ability = NewObject<UUnitWaitAbility>(Unit);
	Ability->InitializeFromDefinition(Definition, Unit);

	FAbilityBattleContext Context;
	Context.SourceUnit = Unit;
	Context.TargetUnits.Add(Target);

	Ability->ConsumeCharge();
	FAbilityValidation Validation = Ability->CanExecute(Context);

	TestFalse("Should fail when no charges", Validation.bIsValid);
	TestEqual("Failure reason should be NoCharges", Validation.FailureReason, EAbilityFailureReason::NoCharges);

	Unit->Destroy();
	Target->Destroy();
	return true;
}

// Test: CanExecute returns failure for active ability without targets
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitAbilityInstanceCanExecuteNoTargetsTest,
	"KBS.Units.Abilities.UnitAbilityInstance.CanExecute_NoTargets_ReturnsFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitAbilityInstanceCanExecuteNoTargetsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitAbilityTestHelper::CreateMockUnit(World);
	UUnitAbilityDefinition* Definition = FUnitAbilityTestHelper::CreateTestAbilityDefinition(Unit);
	Definition->bIsPassive = false;

	UUnitAbilityInstance* Ability = NewObject<UUnitWaitAbility>(Unit);
	Ability->InitializeFromDefinition(Definition, Unit);

	FAbilityBattleContext Context;
	Context.SourceUnit = Unit;

	FAbilityValidation Validation = Ability->CanExecute(Context);

	TestFalse("Should fail when no targets for active ability", Validation.bIsValid);
	TestEqual("Failure reason should be InvalidTarget", Validation.FailureReason, EAbilityFailureReason::InvalidTarget);

	Unit->Destroy();
	return true;
}

// Test: CanExecute returns success when valid
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitAbilityInstanceCanExecuteValidStateTest,
	"KBS.Units.Abilities.UnitAbilityInstance.CanExecute_ValidState_ReturnsSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitAbilityInstanceCanExecuteValidStateTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitAbilityTestHelper::CreateMockUnit(World);
	AUnit* Target = FUnitAbilityTestHelper::CreateMockUnit(World);
	UUnitAbilityDefinition* Definition = FUnitAbilityTestHelper::CreateTestAbilityDefinition(Unit);

	UUnitAbilityInstance* Ability = NewObject<UUnitWaitAbility>(Unit);
	Ability->InitializeFromDefinition(Definition, Unit);

	FAbilityBattleContext Context;
	Context.SourceUnit = Unit;
	Context.TargetUnits.Add(Target);

	FAbilityValidation Validation = Ability->CanExecute(Context);

	TestTrue("Should succeed with valid context", Validation.bIsValid);

	Unit->Destroy();
	Target->Destroy();
	return true;
}

// Test: GetAbilityDisplayData returns correct display data
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitAbilityInstanceGetAbilityDisplayDataTest,
	"KBS.Units.Abilities.UnitAbilityInstance.GetAbilityDisplayData_ReturnsCorrectData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitAbilityInstanceGetAbilityDisplayDataTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitAbilityTestHelper::CreateMockUnit(World);
	UUnitAbilityDefinition* Definition = FUnitAbilityTestHelper::CreateTestAbilityDefinition(Unit);
	Definition->AbilityName = TEXT("Fireball");
	Definition->MaxCharges = 5;

	UUnitAbilityInstance* Ability = NewObject<UUnitWaitAbility>(Unit);
	Ability->InitializeFromDefinition(Definition, Unit);
	Ability->ConsumeCharge();

	FAbilityDisplayData DisplayData = Ability->GetAbilityDisplayData();

	TestEqual("AbilityName should match", DisplayData.AbilityName, TEXT("Fireball"));
	TestEqual("MaxCharges should be 5", DisplayData.MaxCharges, 5);
	TestEqual("RemainingCharges should be 4", DisplayData.RemainingCharges, 4);
	TestTrue("bCanExecuteThisTurn should be true", DisplayData.bCanExecuteThisTurn);
	TestFalse("bIsEmpty should be false", DisplayData.bIsEmpty);

	Unit->Destroy();
	return true;
}

// Test: TriggerAbility validates before execution
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitAbilityInstanceTriggerAbilityValidatesTest,
	"KBS.Units.Abilities.UnitAbilityInstance.TriggerAbility_ValidatesBeforeExecution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitAbilityInstanceTriggerAbilityValidatesTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	AUnit* Unit = FUnitAbilityTestHelper::CreateMockUnit(World);
	UUnitAbilityDefinition* Definition = FUnitAbilityTestHelper::CreateTestAbilityDefinition(Unit);
	Definition->MaxCharges = 1;

	UUnitAbilityInstance* Ability = NewObject<UUnitWaitAbility>(Unit);
	Ability->InitializeFromDefinition(Definition, Unit);

	FAbilityBattleContext Context;
	Context.SourceUnit = Unit;

	Ability->ConsumeCharge();
	FAbilityResult Result = Ability->TriggerAbility(Context);

	TestFalse("Should fail validation (no charges)", Result.bSuccess);
	TestEqual("Failure reason should be NoCharges", Result.FailureReason, EAbilityFailureReason::NoCharges);

	Unit->Destroy();
	return true;
}
