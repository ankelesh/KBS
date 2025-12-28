#include "Misc/AutomationTest.h"
#include "GameMechanics/Units/Abilities/UnitMovementAbility.h"
#include "GameMechanics/Units/Abilities/UnitWaitAbility.h"
#include "GameMechanics/Units/Abilities/UnitFleeAbility.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Abilities/AbilityFactory.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/Components/GridMovementComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridTargetingComponent.h"
#include "GameplayTypes/AbilityBattleContext.h"

class FUtilityAbilitiesTestHelper
{
public:
	static void SetupGridWithComponents(UWorld* World, ATacBattleGrid*& OutGrid, UGridDataManager*& OutDataManager, UGridMovementComponent*& OutMovement, UGridTargetingComponent*& OutTargeting)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		OutGrid = World->SpawnActor<ATacBattleGrid>(ATacBattleGrid::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		OutDataManager = OutGrid->GetDataManager();
		OutDataManager->Initialize(OutGrid);

		OutMovement = OutGrid->GetMovementComponent();
		OutMovement->Initialize(OutGrid, OutDataManager);

		OutTargeting = OutGrid->GetTargetingComponent();
		OutTargeting->Initialize(OutDataManager);
	}

	static UUnitDefinition* CreateTestUnitDefinition(UObject* Outer)
	{
		UUnitDefinition* Definition = NewObject<UUnitDefinition>(Outer);
		Definition->MovementSpeed = 300.0f;
		Definition->UnitName = TEXT("TestUnit");
		Definition->BaseStatsTemplate.MaxHealth = 100.0f;
		return Definition;
	}

	static AUnit* CreateAndPlaceUnit(UWorld* World, UGridDataManager* DataManager, ATacBattleGrid* Grid, int32 Row, int32 Col)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AUnit* Unit = World->SpawnActor<AUnit>(AUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (Unit)
		{
			UUnitDefinition* Definition = CreateTestUnitDefinition(Unit);
			Unit->SetUnitDefinition(Definition);
			DataManager->PlaceUnit(Unit, Row, Col, EBattleLayer::Ground, Grid);
		}
		return Unit;
	}

	static UUnitAbilityDefinition* CreateTestAbilityDefinition(UObject* Outer, TSubclassOf<UUnitAbilityInstance> AbilityClass)
	{
		UUnitAbilityDefinition* Definition = NewObject<UUnitAbilityDefinition>(Outer);
		Definition->AbilityClass = AbilityClass;
		Definition->AbilityName = TEXT("TestAbility");
		Definition->MaxCharges = 3;
		return Definition;
	}
};

// ============================================================================
// UnitMovementAbility Tests
// ============================================================================

// Test: GetTargeting always returns Movement
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitMovementAbilityGetTargetingReturnsMovementTest,
	"KBS.Units.Abilities.UnitMovementAbility.GetTargeting_ReturnsMovement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitMovementAbilityGetTargetingReturnsMovementTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	UGridTargetingComponent* Targeting;
	FUtilityAbilitiesTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement, Targeting);

	AUnit* Unit = FUtilityAbilitiesTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2);

	UUnitAbilityDefinition* Definition = FUtilityAbilitiesTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitMovementAbility::StaticClass());

	UUnitMovementAbility* Ability = Cast<UUnitMovementAbility>(
		UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit));

	TestEqual("Targeting should always be Movement", Ability->GetTargeting(), ETargetReach::Movement);

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: ApplyAbilityEffect moves unit successfully
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitMovementAbilityApplyAbilityEffectMovesUnitTest,
	"KBS.Units.Abilities.UnitMovementAbility.ApplyAbilityEffect_MovesUnit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitMovementAbilityApplyAbilityEffectMovesUnitTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	UGridTargetingComponent* Targeting;
	FUtilityAbilitiesTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement, Targeting);

	AUnit* Unit = FUtilityAbilitiesTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2);

	UUnitAbilityDefinition* Definition = FUtilityAbilitiesTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitMovementAbility::StaticClass());

	UUnitMovementAbility* Ability = Cast<UUnitMovementAbility>(
		UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit));

	FAbilityBattleContext Context;
	Context.SourceUnit = Unit;
	Context.Grid = Grid;
	Context.TargetCell = FIntPoint(3, 2);

	FAbilityResult Result = Ability->ApplyAbilityEffect(Context);

	TestTrue("Should succeed", Result.bSuccess);
	TestEqual("TurnAction should be EndTurn", Result.TurnAction, EAbilityTurnAction::EndTurn);

	int32 Row, Col;
	EBattleLayer Layer;
	DataManager->GetUnitPosition(Unit, Row, Col, Layer);
	TestEqual("Unit should be at new row", Row, 3);
	TestEqual("Unit should be at new col", Col, 2);

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: ApplyAbilityEffect returns failure for invalid cell
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitMovementAbilityApplyAbilityEffectInvalidCellTest,
	"KBS.Units.Abilities.UnitMovementAbility.ApplyAbilityEffect_InvalidCell_ReturnsFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitMovementAbilityApplyAbilityEffectInvalidCellTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	UGridTargetingComponent* Targeting;
	FUtilityAbilitiesTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement, Targeting);

	AUnit* Unit = FUtilityAbilitiesTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2);

	UUnitAbilityDefinition* Definition = FUtilityAbilitiesTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitMovementAbility::StaticClass());

	UUnitMovementAbility* Ability = Cast<UUnitMovementAbility>(
		UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit));

	FAbilityBattleContext Context;
	Context.SourceUnit = Unit;
	Context.Grid = Grid;
	Context.TargetCell = FIntPoint(-1, -1);

	FAbilityResult Result = Ability->ApplyAbilityEffect(Context);

	TestFalse("Should fail with invalid cell", Result.bSuccess);
	TestEqual("Failure reason should be InvalidTarget", Result.FailureReason, EAbilityFailureReason::InvalidTarget);

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: ApplyAbilityEffect returns failure when no grid
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitMovementAbilityApplyAbilityEffectNoGridTest,
	"KBS.Units.Abilities.UnitMovementAbility.ApplyAbilityEffect_NoGrid_ReturnsFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitMovementAbilityApplyAbilityEffectNoGridTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AUnit* Unit = World->SpawnActor<AUnit>(AUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	UUnitDefinition* UnitDef = FUtilityAbilitiesTestHelper::CreateTestUnitDefinition(Unit);
	Unit->SetUnitDefinition(UnitDef);

	UUnitAbilityDefinition* Definition = FUtilityAbilitiesTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitMovementAbility::StaticClass());

	UUnitMovementAbility* Ability = Cast<UUnitMovementAbility>(
		UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit));

	FAbilityBattleContext Context;
	Context.SourceUnit = Unit;
	Context.Grid = nullptr;
	Context.TargetCell = FIntPoint(3, 2);

	FAbilityResult Result = Ability->ApplyAbilityEffect(Context);

	TestFalse("Should fail with no grid", Result.bSuccess);

	Unit->Destroy();
	return true;
}

// Test: CanExecute returns success for valid cell in range
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitMovementAbilityCanExecuteValidCellTest,
	"KBS.Units.Abilities.UnitMovementAbility.CanExecute_ValidCell_ReturnsSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitMovementAbilityCanExecuteValidCellTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	UGridTargetingComponent* Targeting;
	FUtilityAbilitiesTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement, Targeting);

	AUnit* Unit = FUtilityAbilitiesTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2);

	UUnitAbilityDefinition* Definition = FUtilityAbilitiesTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitMovementAbility::StaticClass());

	UUnitMovementAbility* Ability = Cast<UUnitMovementAbility>(
		UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit));

	FAbilityBattleContext Context;
	Context.SourceUnit = Unit;
	Context.Grid = Grid;
	Context.TargetCell = FIntPoint(3, 2);

	FAbilityValidation Validation = Ability->CanExecute(Context);

	TestTrue("Should be valid for cell in range", Validation.bIsValid);

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: CanExecute returns failure for cell with negative coords
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitMovementAbilityCanExecuteNoTargetCellTest,
	"KBS.Units.Abilities.UnitMovementAbility.CanExecute_NoTargetCell_ReturnsFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitMovementAbilityCanExecuteNoTargetCellTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	UGridTargetingComponent* Targeting;
	FUtilityAbilitiesTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement, Targeting);

	AUnit* Unit = FUtilityAbilitiesTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2);

	UUnitAbilityDefinition* Definition = FUtilityAbilitiesTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitMovementAbility::StaticClass());

	UUnitMovementAbility* Ability = Cast<UUnitMovementAbility>(
		UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit));

	FAbilityBattleContext Context;
	Context.SourceUnit = Unit;
	Context.Grid = Grid;
	Context.TargetCell = FIntPoint(-1, -1);

	FAbilityValidation Validation = Ability->CanExecute(Context);

	TestFalse("Should fail with no target cell", Validation.bIsValid);
	TestEqual("Failure reason should be InvalidTarget", Validation.FailureReason, EAbilityFailureReason::InvalidTarget);

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// ============================================================================
// UnitWaitAbility Tests
// ============================================================================

// Test: ApplyAbilityEffect succeeds
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitWaitAbilityApplyAbilityEffectSucceedsTest,
	"KBS.Units.Abilities.UnitWaitAbility.ApplyAbilityEffect_Succeeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitWaitAbilityApplyAbilityEffectSucceedsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AUnit* Unit = World->SpawnActor<AUnit>(AUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	UUnitDefinition* UnitDef = FUtilityAbilitiesTestHelper::CreateTestUnitDefinition(Unit);
	Unit->SetUnitDefinition(UnitDef);

	UUnitAbilityDefinition* Definition = FUtilityAbilitiesTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitWaitAbility::StaticClass());

	UUnitWaitAbility* Ability = Cast<UUnitWaitAbility>(
		UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit));

	FAbilityBattleContext Context;
	Context.SourceUnit = Unit;

	FAbilityResult Result = Ability->ApplyAbilityEffect(Context);

	TestTrue("Should succeed", Result.bSuccess);
	TestEqual("TurnAction should be Wait", Result.TurnAction, EAbilityTurnAction::Wait);
	TestEqual("UnitsAffected should contain source unit", Result.UnitsAffected.Num(), 1);
	TestTrue("UnitsAffected[0] should be source unit", Result.UnitsAffected[0] == Unit);

	Unit->Destroy();
	return true;
}

// Test: ApplyAbilityEffect returns failure when no source unit
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitWaitAbilityApplyAbilityEffectNoSourceTest,
	"KBS.Units.Abilities.UnitWaitAbility.ApplyAbilityEffect_NoSourceUnit_ReturnsFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitWaitAbilityApplyAbilityEffectNoSourceTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AUnit* Unit = World->SpawnActor<AUnit>(AUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	UUnitDefinition* UnitDef = FUtilityAbilitiesTestHelper::CreateTestUnitDefinition(Unit);
	Unit->SetUnitDefinition(UnitDef);

	UUnitAbilityDefinition* Definition = FUtilityAbilitiesTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitWaitAbility::StaticClass());

	UUnitWaitAbility* Ability = Cast<UUnitWaitAbility>(
		UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit));

	FAbilityBattleContext Context;
	Context.SourceUnit = nullptr;

	FAbilityResult Result = Ability->ApplyAbilityEffect(Context);

	TestFalse("Should fail when no source unit", Result.bSuccess);

	Unit->Destroy();
	return true;
}

// ============================================================================
// UnitFleeAbility Tests
// ============================================================================

// Test: ApplyAbilityEffect succeeds
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUnitFleeAbilityApplyAbilityEffectSucceedsTest,
	"KBS.Units.Abilities.UnitFleeAbility.ApplyAbilityEffect_Succeeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FUnitFleeAbilityApplyAbilityEffectSucceedsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AUnit* Unit = World->SpawnActor<AUnit>(AUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	UUnitDefinition* UnitDef = FUtilityAbilitiesTestHelper::CreateTestUnitDefinition(Unit);
	Unit->SetUnitDefinition(UnitDef);

	UUnitAbilityDefinition* Definition = FUtilityAbilitiesTestHelper::CreateTestAbilityDefinition(
		Unit, UUnitFleeAbility::StaticClass());

	UUnitFleeAbility* Ability = Cast<UUnitFleeAbility>(
		UAbilityFactory::CreateAbilityFromDefinition(Definition, Unit));

	FAbilityBattleContext Context;
	Context.SourceUnit = Unit;

	FAbilityResult Result = Ability->ApplyAbilityEffect(Context);

	TestTrue("Should succeed", Result.bSuccess);

	Unit->Destroy();
	return true;
}
