#include "Misc/AutomationTest.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/LargeUnit.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameplayTypes/GridCoordinates.h"

// Helper class for creating test fixtures
class FGridDataManagerTestHelper
{
public:
	static UGridDataManager* CreateDataManager(UWorld* World, ATacBattleGrid* Grid)
	{
		UGridDataManager* DataManager = Grid->GetDataManager();
		DataManager->Initialize(Grid);
		return DataManager;
	}

	static UUnitDefinition* CreateTestUnitDefinition(UObject* Outer)
	{
		UUnitDefinition* Definition = NewObject<UUnitDefinition>(Outer);
		Definition->MovementSpeed = 300.0f;
		Definition->UnitName = TEXT("TestUnit");
		return Definition;
	}

	static AUnit* CreateMockUnit(UWorld* World, ETeamSide TeamSide = ETeamSide::Attacker, bool bIsMultiCell = false)
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
		}
		return Unit;
	}

	static ATacBattleGrid* CreateMockGrid(UWorld* World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return World->SpawnActor<ATacBattleGrid>(ATacBattleGrid::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}
};

// Test: Initialization creates layers and teams
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerInitializeTest,
	"KBS.Grid.Components.DataManager.Initialize_CreatesLayersAndTeams",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerInitializeTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError("Failed to get world");
		return false;
	}

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);

	TestNotNull("Grid should be set", Grid);
	TestNotNull("AttackerTeam should be created", DataManager->GetAttackerTeam());
	TestNotNull("DefenderTeam should be created", DataManager->GetDefenderTeam());
	TestEqual("AttackerTeam side", DataManager->GetAttackerTeam()->GetTeamSide(), ETeamSide::Attacker);
	TestEqual("DefenderTeam side", DataManager->GetDefenderTeam()->GetTeamSide(), ETeamSide::Defender);

	Grid->Destroy();
	return true;
}

// Test: GetLayer returns correct layer
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetLayerTest,
	"KBS.Grid.Components.DataManager.GetLayer_ReturnsCorrectLayer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetLayerTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);

	const TArray<FGridRow>& GroundLayer = DataManager->GetLayer(EBattleLayer::Ground);
	const TArray<FGridRow>& AirLayer = DataManager->GetLayer(EBattleLayer::Air);

	TestEqual("Ground layer size", GroundLayer.Num(), FGridCoordinates::GridSize);
	TestEqual("Air layer size", AirLayer.Num(), FGridCoordinates::GridSize);

	Grid->Destroy();
	return true;
}

// Test: PlaceUnit on valid cell succeeds
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerPlaceUnitValidTest,
	"KBS.Grid.Components.DataManager.PlaceUnit_ValidCell_Succeeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerPlaceUnitValidTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Unit = FGridDataManagerTestHelper::CreateMockUnit(World);

	bool bPlaced = DataManager->PlaceUnit(Unit, 2, 2, EBattleLayer::Ground, Grid);
	TestTrue("Unit should be placed", bPlaced);

	AUnit* RetrievedUnit = DataManager->GetUnit(2, 2, EBattleLayer::Ground);
	TestEqual("Retrieved unit should match placed unit", RetrievedUnit, Unit);

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: PlaceUnit on occupied cell fails
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerPlaceUnitOccupiedTest,
	"KBS.Grid.Components.DataManager.PlaceUnit_OccupiedCell_Fails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerPlaceUnitOccupiedTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Unit1 = FGridDataManagerTestHelper::CreateMockUnit(World);
	AUnit* Unit2 = FGridDataManagerTestHelper::CreateMockUnit(World);

	DataManager->PlaceUnit(Unit1, 2, 2, EBattleLayer::Ground, Grid);
	bool bPlaced = DataManager->PlaceUnit(Unit2, 2, 2, EBattleLayer::Ground, Grid);

	TestFalse("Placing on occupied cell should fail", bPlaced);

	Grid->Destroy();
	Unit1->Destroy();
	Unit2->Destroy();
	return true;
}

// Test: PlaceUnit on invalid cell fails
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerPlaceUnitInvalidTest,
	"KBS.Grid.Components.DataManager.PlaceUnit_InvalidCell_Fails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerPlaceUnitInvalidTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Unit = FGridDataManagerTestHelper::CreateMockUnit(World);

	TestFalse("Negative row should fail", DataManager->PlaceUnit(Unit, -1, 2, EBattleLayer::Ground, Grid));
	TestFalse("Out of bounds row should fail", DataManager->PlaceUnit(Unit, 10, 2, EBattleLayer::Ground, Grid));
	TestFalse("Out of bounds col should fail", DataManager->PlaceUnit(Unit, 2, 10, EBattleLayer::Ground, Grid));
	TestFalse("Restricted cell should fail", DataManager->PlaceUnit(Unit, 2, 0, EBattleLayer::Ground, Grid));

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: PlaceUnit multi-cell occupies both cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerPlaceMultiCellTest,
	"KBS.Grid.Components.DataManager.PlaceUnit_MultiCellUnit_BothCellsOccupied",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerPlaceMultiCellTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* MultiUnit = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Attacker, true);

	bool bPlaced = DataManager->PlaceUnit(MultiUnit, 2, 2, EBattleLayer::Ground, Grid);
	TestTrue("Multi-cell unit should be placed", bPlaced);

	TestTrue("Primary cell should be occupied", DataManager->IsCellOccupied(2, 2, EBattleLayer::Ground));
	TestTrue("Unit should be marked as multi-cell", DataManager->IsMultiCellUnit(MultiUnit));

	const FMultiCellUnitData* MultiData = DataManager->GetMultiCellData(MultiUnit);
	TestNotNull("Multi-cell data should exist", MultiData);
	if (MultiData)
	{
		TestEqual("Should occupy 2 cells", MultiData->OccupiedCells.Num(), 2);
	}

	Grid->Destroy();
	MultiUnit->Destroy();
	return true;
}

// Test: PlaceUnit multi-cell fails if secondary occupied
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerPlaceMultiCellBlockedTest,
	"KBS.Grid.Components.DataManager.PlaceUnit_MultiCellUnit_SecondaryOccupied_Fails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerPlaceMultiCellBlockedTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* BlockingUnit = FGridDataManagerTestHelper::CreateMockUnit(World);
	AUnit* MultiUnit = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Attacker, true);

	DataManager->PlaceUnit(BlockingUnit, 1, 2, EBattleLayer::Ground, Grid);

	bool bPlaced = DataManager->PlaceUnit(MultiUnit, 0, 2, EBattleLayer::Ground, Grid);
	TestFalse("Multi-cell placement should fail when secondary cell occupied", bPlaced);

	Grid->Destroy();
	BlockingUnit->Destroy();
	MultiUnit->Destroy();
	return true;
}

// Test: GetUnit from valid position returns unit
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetUnitValidTest,
	"KBS.Grid.Components.DataManager.GetUnit_ValidPosition_ReturnsUnit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetUnitValidTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Unit = FGridDataManagerTestHelper::CreateMockUnit(World);

	DataManager->PlaceUnit(Unit, 1, 3, EBattleLayer::Ground, Grid);
	AUnit* Retrieved = DataManager->GetUnit(1, 3, EBattleLayer::Ground);

	TestEqual("Retrieved unit should match", Retrieved, Unit);

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: GetUnit from empty cell returns null
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetUnitEmptyTest,
	"KBS.Grid.Components.DataManager.GetUnit_EmptyCell_ReturnsNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetUnitEmptyTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);

	AUnit* Retrieved = DataManager->GetUnit(2, 2, EBattleLayer::Ground);
	TestNull("Empty cell should return null", Retrieved);

	Grid->Destroy();
	return true;
}

// Test: GetUnit from invalid cell returns null
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetUnitInvalidTest,
	"KBS.Grid.Components.DataManager.GetUnit_InvalidCell_ReturnsNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetUnitInvalidTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);

	TestNull("Negative coords should return null", DataManager->GetUnit(-1, 2, EBattleLayer::Ground));
	TestNull("Out of bounds should return null", DataManager->GetUnit(10, 10, EBattleLayer::Ground));

	Grid->Destroy();
	return true;
}

// Test: RemoveUnit from valid position succeeds
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerRemoveUnitValidTest,
	"KBS.Grid.Components.DataManager.RemoveUnit_ValidUnit_Succeeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerRemoveUnitValidTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Unit = FGridDataManagerTestHelper::CreateMockUnit(World);

	DataManager->PlaceUnit(Unit, 2, 2, EBattleLayer::Ground, Grid);
	bool bRemoved = DataManager->RemoveUnit(2, 2, EBattleLayer::Ground, Grid);

	TestTrue("Remove should succeed", bRemoved);
	TestNull("Cell should be empty after removal", DataManager->GetUnit(2, 2, EBattleLayer::Ground));

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: RemoveUnit from empty cell fails
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerRemoveUnitEmptyTest,
	"KBS.Grid.Components.DataManager.RemoveUnit_EmptyCell_Fails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerRemoveUnitEmptyTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);

	bool bRemoved = DataManager->RemoveUnit(2, 2, EBattleLayer::Ground, Grid);
	TestFalse("Remove from empty cell should fail", bRemoved);

	Grid->Destroy();
	return true;
}

// Test: RemoveUnit multi-cell clears both cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerRemoveMultiCellTest,
	"KBS.Grid.Components.DataManager.RemoveUnit_MultiCellUnit_BothCellsCleared",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerRemoveMultiCellTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* MultiUnit = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Attacker, true);

	DataManager->PlaceUnit(MultiUnit, 2, 2, EBattleLayer::Ground, Grid);
	const FMultiCellUnitData* MultiData = DataManager->GetMultiCellData(MultiUnit);

	bool bRemoved = DataManager->RemoveUnit(2, 2, EBattleLayer::Ground, Grid);
	TestTrue("Remove should succeed", bRemoved);

	if (MultiData && MultiData->OccupiedCells.Num() == 2)
	{
		for (const FGridCellCoord& Cell : MultiData->OccupiedCells)
		{
			TestNull("All occupied cells should be cleared", DataManager->GetUnit(Cell.Row, Cell.Col, Cell.Layer));
		}
	}

	TestFalse("Unit should no longer be multi-cell tracked", DataManager->IsMultiCellUnit(MultiUnit));

	Grid->Destroy();
	MultiUnit->Destroy();
	return true;
}

// Test: RemoveUnit clears flank and rotation data
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerRemoveUnitClearsDataTest,
	"KBS.Grid.Components.DataManager.RemoveUnit_ClearsFlankAndRotationData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerRemoveUnitClearsDataTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Unit = FGridDataManagerTestHelper::CreateMockUnit(World);

	DataManager->PlaceUnit(Unit, 2, 2, EBattleLayer::Ground, Grid);
	DataManager->SetUnitFlankState(Unit, true);
	DataManager->SetUnitOriginalRotation(Unit, FRotator(0, 90, 0));

	DataManager->RemoveUnit(2, 2, EBattleLayer::Ground, Grid);

	TestFalse("Flank state should be cleared", DataManager->IsUnitOnFlank(Unit));
	TestEqual("Rotation should be cleared", DataManager->GetUnitOriginalRotation(Unit), FRotator::ZeroRotator);

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: GetUnitPosition for single-cell unit
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetUnitPositionSingleTest,
	"KBS.Grid.Components.DataManager.GetUnitPosition_SingleCellUnit_ReturnsCorrectPosition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetUnitPositionSingleTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Unit = FGridDataManagerTestHelper::CreateMockUnit(World);

	DataManager->PlaceUnit(Unit, 3, 1, EBattleLayer::Air, Grid);

	int32 Row, Col;
	EBattleLayer Layer;
	bool bFound = DataManager->GetUnitPosition(Unit, Row, Col, Layer);

	TestTrue("Should find unit position", bFound);
	TestEqual("Row should match", Row, 3);
	TestEqual("Col should match", Col, 1);
	TestEqual("Layer should match", Layer, EBattleLayer::Air);

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: GetUnitPosition for multi-cell unit returns primary
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetUnitPositionMultiTest,
	"KBS.Grid.Components.DataManager.GetUnitPosition_MultiCellUnit_ReturnsPrimaryCell",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetUnitPositionMultiTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* MultiUnit = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Attacker, true);

	DataManager->PlaceUnit(MultiUnit, 2, 2, EBattleLayer::Ground, Grid);

	int32 Row, Col;
	EBattleLayer Layer;
	bool bFound = DataManager->GetUnitPosition(MultiUnit, Row, Col, Layer);

	TestTrue("Should find multi-cell unit position", bFound);
	TestEqual("Row should be primary", Row, 2);
	TestEqual("Col should be primary", Col, 2);

	Grid->Destroy();
	MultiUnit->Destroy();
	return true;
}

// Test: GetUnitPosition for unplaced unit returns false
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetUnitPositionNotPlacedTest,
	"KBS.Grid.Components.DataManager.GetUnitPosition_NotPlacedUnit_ReturnsFalse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetUnitPositionNotPlacedTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Unit = FGridDataManagerTestHelper::CreateMockUnit(World);

	int32 Row, Col;
	EBattleLayer Layer;
	bool bFound = DataManager->GetUnitPosition(Unit, Row, Col, Layer);

	TestFalse("Unplaced unit should not be found", bFound);

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: Corpse push adds corpse to stack
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerPushCorpseTest,
	"KBS.Grid.Components.DataManager.PushCorpse_ValidCell_AddsCorpse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerPushCorpseTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Corpse = FGridDataManagerTestHelper::CreateMockUnit(World);

	DataManager->PushCorpse(Corpse, 2, 2);

	TestTrue("Should have corpses", DataManager->HasCorpses(2, 2));
	TestEqual("Top corpse should match", DataManager->GetTopCorpse(2, 2), Corpse);

	Grid->Destroy();
	Corpse->Destroy();
	return true;
}

// Test: Multiple corpses stack correctly (LIFO)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerPushMultipleCorpsesTest,
	"KBS.Grid.Components.DataManager.PushCorpse_MultipleCorpses_StacksCorrectly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerPushMultipleCorpsesTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Corpse1 = FGridDataManagerTestHelper::CreateMockUnit(World);
	AUnit* Corpse2 = FGridDataManagerTestHelper::CreateMockUnit(World);
	AUnit* Corpse3 = FGridDataManagerTestHelper::CreateMockUnit(World);

	DataManager->PushCorpse(Corpse1, 2, 2);
	DataManager->PushCorpse(Corpse2, 2, 2);
	DataManager->PushCorpse(Corpse3, 2, 2);

	TestEqual("Top corpse should be last pushed", DataManager->GetTopCorpse(2, 2), Corpse3);
	const TArray<TObjectPtr<AUnit>>& Stack = DataManager->GetCorpseStack(2, 2);
	TestEqual("Stack should have 3 corpses", Stack.Num(), 3);

	Grid->Destroy();
	Corpse1->Destroy();
	Corpse2->Destroy();
	Corpse3->Destroy();
	return true;
}

// Test: PopCorpse removes top corpse
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerPopCorpseTest,
	"KBS.Grid.Components.DataManager.PopCorpse_RemovesTopCorpse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerPopCorpseTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Corpse1 = FGridDataManagerTestHelper::CreateMockUnit(World);
	AUnit* Corpse2 = FGridDataManagerTestHelper::CreateMockUnit(World);

	DataManager->PushCorpse(Corpse1, 2, 2);
	DataManager->PushCorpse(Corpse2, 2, 2);

	AUnit* Popped = DataManager->PopCorpse(2, 2);
	TestEqual("Popped corpse should be Corpse2", Popped, Corpse2);
	TestEqual("New top should be Corpse1", DataManager->GetTopCorpse(2, 2), Corpse1);

	Grid->Destroy();
	Corpse1->Destroy();
	Corpse2->Destroy();
	return true;
}

// Test: PopCorpse from empty stack returns null
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerPopCorpseEmptyTest,
	"KBS.Grid.Components.DataManager.PopCorpse_EmptyStack_ReturnsNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerPopCorpseEmptyTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);

	AUnit* Popped = DataManager->PopCorpse(2, 2);
	TestNull("Pop from empty stack should return null", Popped);

	Grid->Destroy();
	return true;
}

// Test: GetTopCorpse without removing
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetTopCorpseTest,
	"KBS.Grid.Components.DataManager.GetTopCorpse_ReturnsCorrectCorpse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetTopCorpseTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Corpse = FGridDataManagerTestHelper::CreateMockUnit(World);

	DataManager->PushCorpse(Corpse, 2, 2);

	AUnit* Top1 = DataManager->GetTopCorpse(2, 2);
	AUnit* Top2 = DataManager->GetTopCorpse(2, 2);

	TestEqual("GetTopCorpse should not remove", Top1, Top2);
	TestEqual("Should still be on stack", Top1, Corpse);

	Grid->Destroy();
	Corpse->Destroy();
	return true;
}

// Test: HasCorpses returns correct state
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerHasCorpsesTest,
	"KBS.Grid.Components.DataManager.HasCorpses_ReturnsCorrectState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerHasCorpsesTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Corpse = FGridDataManagerTestHelper::CreateMockUnit(World);

	TestFalse("Empty cell has no corpses", DataManager->HasCorpses(2, 2));

	DataManager->PushCorpse(Corpse, 2, 2);
	TestTrue("Cell with corpse returns true", DataManager->HasCorpses(2, 2));

	Grid->Destroy();
	Corpse->Destroy();
	return true;
}

// Test: GetTeamForUnit returns correct team
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetTeamForUnitTest,
	"KBS.Grid.Components.DataManager.GetTeamForUnit_ReturnsCorrectTeam",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetTeamForUnitTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* AttackerUnit = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Attacker);
	AUnit* DefenderUnit = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Defender);

	DataManager->GetAttackerTeam()->AddUnit(AttackerUnit);
	DataManager->GetDefenderTeam()->AddUnit(DefenderUnit);

	TestEqual("Attacker unit returns attacker team", DataManager->GetTeamForUnit(AttackerUnit), DataManager->GetAttackerTeam());
	TestEqual("Defender unit returns defender team", DataManager->GetTeamForUnit(DefenderUnit), DataManager->GetDefenderTeam());

	Grid->Destroy();
	AttackerUnit->Destroy();
	DefenderUnit->Destroy();
	return true;
}

// Test: GetTeamForUnit unregistered returns null
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetTeamForUnregisteredTest,
	"KBS.Grid.Components.DataManager.GetTeamForUnit_UnregisteredUnit_ReturnsNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetTeamForUnregisteredTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Unit = FGridDataManagerTestHelper::CreateMockUnit(World);

	TestNull("Unregistered unit should return null", DataManager->GetTeamForUnit(Unit));

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: GetEnemyTeam returns opposite team
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetEnemyTeamTest,
	"KBS.Grid.Components.DataManager.GetEnemyTeam_ReturnsOppositeTeam",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetEnemyTeamTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* AttackerUnit = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Attacker);

	DataManager->GetAttackerTeam()->AddUnit(AttackerUnit);

	TestEqual("Attacker's enemy is defender", DataManager->GetEnemyTeam(AttackerUnit), DataManager->GetDefenderTeam());

	Grid->Destroy();
	AttackerUnit->Destroy();
	return true;
}

// Test: GetUnitsFromTeam returns correct units
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetUnitsFromTeamTest,
	"KBS.Grid.Components.DataManager.GetUnitsFromTeam_ReturnsCorrectUnits",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetUnitsFromTeamTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Unit1 = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Attacker);
	AUnit* Unit2 = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Attacker);

	DataManager->GetAttackerTeam()->AddUnit(Unit1);
	DataManager->GetAttackerTeam()->AddUnit(Unit2);

	TArray<AUnit*> AttackerUnits = DataManager->GetUnitsFromTeam(true);
	TestEqual("Should return 2 attacker units", AttackerUnits.Num(), 2);

	Grid->Destroy();
	Unit1->Destroy();
	Unit2->Destroy();
	return true;
}

// Test: GetEmptyCells returns only empty cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetEmptyCellsTest,
	"KBS.Grid.Components.DataManager.GetEmptyCells_ReturnsOnlyEmptyCells",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetEmptyCellsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Unit = FGridDataManagerTestHelper::CreateMockUnit(World);

	TArray<FIntPoint> EmptyBefore = DataManager->GetEmptyCells(EBattleLayer::Ground);
	int32 EmptyCountBefore = EmptyBefore.Num();

	DataManager->PlaceUnit(Unit, 2, 2, EBattleLayer::Ground, Grid);
	TArray<FIntPoint> EmptyAfter = DataManager->GetEmptyCells(EBattleLayer::Ground);

	TestEqual("Empty cells should decrease by 1", EmptyAfter.Num(), EmptyCountBefore - 1);
	TestFalse("Occupied cell should not be in empty list", EmptyAfter.Contains(FIntPoint(2, 2)));

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: GetOccupiedCells returns only team cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetOccupiedCellsTest,
	"KBS.Grid.Components.DataManager.GetOccupiedCells_ReturnsOnlyOccupiedByTeam",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetOccupiedCellsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* AttackerUnit = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Attacker);
	AUnit* DefenderUnit = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Defender);

	DataManager->GetAttackerTeam()->AddUnit(AttackerUnit);
	DataManager->GetDefenderTeam()->AddUnit(DefenderUnit);
	DataManager->PlaceUnit(AttackerUnit, 1, 1, EBattleLayer::Ground, Grid);
	DataManager->PlaceUnit(DefenderUnit, 3, 3, EBattleLayer::Ground, Grid);

	TArray<FIntPoint> AttackerCells = DataManager->GetOccupiedCells(EBattleLayer::Ground, DataManager->GetAttackerTeam());
	TestEqual("Should have 1 attacker cell", AttackerCells.Num(), 1);
	TestTrue("Should contain attacker cell", AttackerCells.Contains(FIntPoint(1, 1)));
	TestFalse("Should not contain defender cell", AttackerCells.Contains(FIntPoint(3, 3)));

	Grid->Destroy();
	AttackerUnit->Destroy();
	DefenderUnit->Destroy();
	return true;
}

// Test: IsCellOccupied returns correct state
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerIsCellOccupiedTest,
	"KBS.Grid.Components.DataManager.IsCellOccupied_ReturnsCorrectState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerIsCellOccupiedTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Unit = FGridDataManagerTestHelper::CreateMockUnit(World);

	TestFalse("Empty cell not occupied", DataManager->IsCellOccupied(2, 2, EBattleLayer::Ground));

	DataManager->PlaceUnit(Unit, 2, 2, EBattleLayer::Ground, Grid);
	TestTrue("Occupied cell returns true", DataManager->IsCellOccupied(2, 2, EBattleLayer::Ground));

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: GetValidPlacementCells excludes restricted
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetValidPlacementCellsTest,
	"KBS.Grid.Components.DataManager.GetValidPlacementCells_ExcludesRestricted",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetValidPlacementCellsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);

	TArray<FIntPoint> ValidCells = DataManager->GetValidPlacementCells(EBattleLayer::Ground);

	TestFalse("Restricted (2,0) should not be included", ValidCells.Contains(FIntPoint(0, 2)));
	TestFalse("Restricted (2,4) should not be included", ValidCells.Contains(FIntPoint(4, 2)));

	Grid->Destroy();
	return true;
}

// Test: IsMultiCellUnit returns correct value
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerIsMultiCellUnitTest,
	"KBS.Grid.Components.DataManager.IsMultiCellUnit_ReturnsCorrectValue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerIsMultiCellUnitTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* SingleUnit = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Attacker, false);
	AUnit* MultiUnit = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Attacker, true);

	DataManager->PlaceUnit(SingleUnit, 1, 1, EBattleLayer::Ground, Grid);
	DataManager->PlaceUnit(MultiUnit, 3, 3, EBattleLayer::Ground, Grid);

	TestFalse("Single-cell unit returns false", DataManager->IsMultiCellUnit(SingleUnit));
	TestTrue("Multi-cell unit returns true", DataManager->IsMultiCellUnit(MultiUnit));

	Grid->Destroy();
	SingleUnit->Destroy();
	MultiUnit->Destroy();
	return true;
}

// Test: GetMultiCellData returns correct data
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetMultiCellDataTest,
	"KBS.Grid.Components.DataManager.GetMultiCellData_ReturnsCorrectData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetMultiCellDataTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* MultiUnit = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Attacker, true);

	DataManager->PlaceUnit(MultiUnit, 2, 2, EBattleLayer::Ground, Grid);

	const FMultiCellUnitData* MultiData = DataManager->GetMultiCellData(MultiUnit);
	TestNotNull("Multi-cell data should exist", MultiData);

	if (MultiData)
	{
		TestEqual("Should have 2 occupied cells", MultiData->OccupiedCells.Num(), 2);
		TestEqual("Primary cell row", MultiData->PrimaryCell.Row, 2);
		TestEqual("Primary cell col", MultiData->PrimaryCell.Col, 2);
	}

	Grid->Destroy();
	MultiUnit->Destroy();
	return true;
}
