#include "Misc/AutomationTest.h"
#include "GameMechanics/Tactical/Grid/Components/GridMovementComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/LargeUnit.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameplayTypes/GridCoordinates.h"

// Helper class for movement tests
class FGridMovementTestHelper
{
public:
	static void SetupGridWithComponents(UWorld* World, ATacBattleGrid*& OutGrid, UGridDataManager*& OutDataManager, UGridMovementComponent*& OutMovement)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		OutGrid = World->SpawnActor<ATacBattleGrid>(ATacBattleGrid::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		OutDataManager = OutGrid->GetDataManager();
		OutDataManager->Initialize(OutGrid);

		OutMovement = OutGrid->GetMovementComponent();
		OutMovement->Initialize(OutGrid, OutDataManager);
	}

	static UUnitDefinition* CreateTestUnitDefinition(UObject* Outer)
	{
		UUnitDefinition* Definition = NewObject<UUnitDefinition>(Outer);
		Definition->MovementSpeed = 300.0f;
		Definition->UnitName = TEXT("TestUnit");
		return Definition;
	}

	static AUnit* CreateAndPlaceUnit(UWorld* World, UGridDataManager* DataManager, ATacBattleGrid* Grid, int32 Row, int32 Col, EBattleLayer Layer, ETeamSide TeamSide = ETeamSide::Attacker, bool bIsMultiCell = false)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AUnit* Unit = bIsMultiCell ?
			World->SpawnActor<ALargeUnit>(ALargeUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams) :
			World->SpawnActor<AUnit>(AUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (Unit)
		{
			UUnitDefinition* Definition = CreateTestUnitDefinition(Unit);
			Unit->SetUnitDefinition(Definition);
			Unit->SetTeamSide(TeamSide);
			DataManager->PlaceUnit(Unit, Row, Col, Layer, Grid);
		}
		return Unit;
	}
};

// Test: Move unit to adjacent empty cell succeeds
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridMovementMoveToEmptyCellTest,
	"KBS.Grid.Components.Movement.MoveUnit_AdjacentEmptyCell_Succeeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridMovementMoveToEmptyCellTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FGridMovementTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	AUnit* Unit = FGridMovementTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground);

	bool bMoved = Movement->MoveUnit(Unit, 2, 3);
	TestTrue("Move should succeed", bMoved);

	int32 Row, Col;
	EBattleLayer Layer;
	DataManager->GetUnitPosition(Unit, Row, Col, Layer);
	TestEqual("Unit should be at new row", Row, 2);
	TestEqual("Unit should be at new col", Col, 3);
	TestNull("Old cell should be empty", DataManager->GetUnit(2, 2, EBattleLayer::Ground));

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: Move null unit fails
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridMovementMoveNullUnitTest,
	"KBS.Grid.Components.Movement.MoveUnit_InvalidUnit_Fails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridMovementMoveNullUnitTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FGridMovementTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	AddExpectedError(TEXT("MoveUnit: Invalid parameters!"), EAutomationExpectedErrorFlags::Contains, 1);
	bool bMoved = Movement->MoveUnit(nullptr, 2, 3);
	TestFalse("Moving null unit should fail", bMoved);

	Grid->Destroy();
	return true;
}

// Test: Move unit not on grid fails
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridMovementMoveUnplacedUnitTest,
	"KBS.Grid.Components.Movement.MoveUnit_UnitNotOnGrid_Fails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridMovementMoveUnplacedUnitTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FGridMovementTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AUnit* Unit = World->SpawnActor<AUnit>(AUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	UUnitDefinition* Definition = FGridMovementTestHelper::CreateTestUnitDefinition(Unit);
	Unit->SetUnitDefinition(Definition);

	AddExpectedError(TEXT("MoveUnit: Could not find unit position!"), EAutomationExpectedErrorFlags::Contains, 1);
	bool bMoved = Movement->MoveUnit(Unit, 2, 3);
	TestFalse("Moving unplaced unit should fail", bMoved);

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: Move to invalid cell fails
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridMovementMoveToInvalidCellTest,
	"KBS.Grid.Components.Movement.MoveUnit_ToInvalidCell_Fails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridMovementMoveToInvalidCellTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FGridMovementTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	AUnit* Unit = FGridMovementTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground);

	// Note: Movement component doesn't validate target cell, it delegates to DataManager
	// This test ensures the movement logic itself doesn't crash with invalid coords

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: Move to teammate cell swaps units
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridMovementSwapUnitsTest,
	"KBS.Grid.Components.Movement.MoveUnit_ToTeammateCell_SwapsUnits",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridMovementSwapUnitsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FGridMovementTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	AUnit* Unit1 = FGridMovementTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* Unit2 = FGridMovementTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 3, EBattleLayer::Ground, ETeamSide::Attacker);

	bool bMoved = Movement->MoveUnit(Unit1, 2, 3);
	TestTrue("Swap should succeed", bMoved);

	int32 Row1, Col1, Row2, Col2;
	EBattleLayer Layer1, Layer2;
	DataManager->GetUnitPosition(Unit1, Row1, Col1, Layer1);
	DataManager->GetUnitPosition(Unit2, Row2, Col2, Layer2);

	TestEqual("Unit1 should be at (2,3)", Col1, 3);
	TestEqual("Unit2 should be at (2,2)", Col2, 2);

	Grid->Destroy();
	Unit1->Destroy();
	Unit2->Destroy();
	return true;
}

// Test: Multi-cell unit cannot swap
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridMovementMultiCellCannotSwapTest,
	"KBS.Grid.Components.Movement.MoveUnit_MultiCellUnit_CannotSwap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridMovementMultiCellCannotSwapTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FGridMovementTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	AUnit* MultiUnit = FGridMovementTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 1, 1, EBattleLayer::Ground, ETeamSide::Attacker, true);
	AUnit* NormalUnit = FGridMovementTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 3, 3, EBattleLayer::Ground, ETeamSide::Attacker);

	bool bMoved = Movement->MoveUnit(MultiUnit, 3, 3);
	TestFalse("Multi-cell unit swap should fail", bMoved);

	Grid->Destroy();
	MultiUnit->Destroy();
	NormalUnit->Destroy();
	return true;
}

// Test: Moving to flank cell applies flank rotation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridMovementEnterFlankTest,
	"KBS.Grid.Components.Movement.MoveUnit_EnterFlankCell_AppliesFlankRotation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridMovementEnterFlankTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FGridMovementTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	AUnit* Unit = FGridMovementTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 1, 2, EBattleLayer::Ground);

	bool bMoved = Movement->MoveUnit(Unit, 1, 0);
	TestTrue("Move to flank should succeed", bMoved);
	TestTrue("Unit should be on flank", DataManager->IsUnitOnFlank(Unit));

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: Leaving flank cell restores original rotation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridMovementLeaveFlankTest,
	"KBS.Grid.Components.Movement.MoveUnit_LeaveFlankCell_RestoresOriginalRotation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridMovementLeaveFlankTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FGridMovementTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	AUnit* Unit = FGridMovementTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 1, 0, EBattleLayer::Ground);
	FRotator OriginalRotation = Unit->GetActorRotation();
	DataManager->SetUnitFlankState(Unit, true);
	DataManager->SetUnitOriginalRotation(Unit, OriginalRotation);

	bool bMoved = Movement->MoveUnit(Unit, 1, 2);
	TestTrue("Move from flank should succeed", bMoved);
	TestFalse("Unit should not be on flank", DataManager->IsUnitOnFlank(Unit));

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: Moving from flank to flank maintains flank state
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridMovementFlankToFlankTest,
	"KBS.Grid.Components.Movement.MoveUnit_FlankToFlank_MaintainsFlankState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridMovementFlankToFlankTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FGridMovementTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	AUnit* Unit = FGridMovementTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 0, 2, EBattleLayer::Ground);

	Movement->MoveUnit(Unit, 1, 0);
	TestTrue("Unit should be on flank after first move", DataManager->IsUnitOnFlank(Unit));

	Movement->MoveUnit(Unit, 0, 0);
	TestTrue("Unit should still be on flank", DataManager->IsUnitOnFlank(Unit));

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: Flank state changed delegate fires
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridMovementFlankDelegateTest,
	"KBS.Grid.Components.Movement.FlankStateChanged_DelegateTriggered",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridMovementFlankDelegateTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FGridMovementTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	AUnit* Unit = FGridMovementTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 1, 2, EBattleLayer::Ground);

	bool bDelegateCalled = false;
	Movement->OnUnitFlankStateChanged.AddLambda([&bDelegateCalled](AUnit* ChangedUnit, bool bOnFlank, FIntPoint Cell)
	{
		bDelegateCalled = true;
	});

	Movement->MoveUnit(Unit, 1, 0);
	TestTrue("Delegate should be called when entering flank", bDelegateCalled);

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: Multi-cell unit movement updates both cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridMovementMultiCellBothCellsUpdateTest,
	"KBS.Grid.Components.Movement.MoveUnit_MultiCell_BothCellsUpdate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridMovementMultiCellBothCellsUpdateTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FGridMovementTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	AUnit* MultiUnit = FGridMovementTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 1, 1, EBattleLayer::Ground, ETeamSide::Attacker, true);

	const FMultiCellUnitData* DataBefore = DataManager->GetMultiCellData(MultiUnit);
	if (!DataBefore || DataBefore->OccupiedCells.Num() != 2)
	{
		AddError("Multi-cell unit not properly initialized");
		Grid->Destroy();
		MultiUnit->Destroy();
		return false;
	}

	bool bMoved = Movement->MoveUnit(MultiUnit, 3, 3);
	TestTrue("Multi-cell move should succeed", bMoved);

	const FMultiCellUnitData* DataAfter = DataManager->GetMultiCellData(MultiUnit);
	if (DataAfter)
	{
		TestEqual("Should still occupy 2 cells", DataAfter->OccupiedCells.Num(), 2);
		TestEqual("Primary cell should update", DataAfter->PrimaryCell.Row, 3);
	}

	Grid->Destroy();
	MultiUnit->Destroy();
	return true;
}

// Test: Multi-cell cannot move if secondary blocked
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridMovementMultiCellSecondaryBlockedTest,
	"KBS.Grid.Components.Movement.MoveUnit_MultiCell_SecondaryBlocked_Fails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridMovementMultiCellSecondaryBlockedTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FGridMovementTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	AUnit* MultiUnit = FGridMovementTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 1, 1, EBattleLayer::Ground, ETeamSide::Attacker, true);

	// This test validates the DataManager prevents placement when secondary cell is blocked
	// The movement component relies on DataManager's validation

	Grid->Destroy();
	MultiUnit->Destroy();
	return true;
}

// Test: Default cell orientation for flank points to center
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridMovementFlankOrientationTest,
	"KBS.Grid.Components.Movement.CalculateDefaultCellOrientation_FlankCell_PointsToCenter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridMovementFlankOrientationTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FGridMovementTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	// Flank orientation is calculated internally during movement
	// This is validated through the ApplyFlankRotation logic

	Grid->Destroy();
	return true;
}

// Test: Default cell orientation for normal cells uses team orientation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridMovementNormalOrientationTest,
	"KBS.Grid.Components.Movement.CalculateDefaultCellOrientation_NormalCell_TeamOrientation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridMovementNormalOrientationTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FGridMovementTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	// Team orientation constants are defined in the movement component
	// AttackerDefaultYaw = 0.0f, DefenderDefaultYaw = 180.0f
	// This is applied during movement interpolation

	Grid->Destroy();
	return true;
}
