#include "Misc/AutomationTest.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Units/Unit.h"
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

	static AUnit* CreateMockUnit(UWorld* World, ETeamSide TeamSide = ETeamSide::Attacker)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AUnit* Unit = World->SpawnActor<AUnit>(AUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

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

	bool bPlaced = DataManager->PlaceUnit(Unit, 2, 2, ETacGridLayer::Ground);
	TestTrue("Unit should be placed", bPlaced);

	AUnit* RetrievedUnit = DataManager->GetUnit(2, 2, ETacGridLayer::Ground);
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

	DataManager->PlaceUnit(Unit1, 2, 2, ETacGridLayer::Ground);
	bool bPlaced = DataManager->PlaceUnit(Unit2, 2, 2, ETacGridLayer::Ground);

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

	TestFalse("Negative row should fail", DataManager->PlaceUnit(Unit, -1, 2, ETacGridLayer::Ground));
	TestFalse("Out of bounds row should fail", DataManager->PlaceUnit(Unit, 10, 2, ETacGridLayer::Ground));
	TestFalse("Out of bounds col should fail", DataManager->PlaceUnit(Unit, 2, 10, ETacGridLayer::Ground));
	TestFalse("Restricted cell should fail", DataManager->PlaceUnit(Unit, 2, 0, ETacGridLayer::Ground));

	Grid->Destroy();
	Unit->Destroy();
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

	DataManager->PlaceUnit(Unit, 1, 3, ETacGridLayer::Ground);
	AUnit* Retrieved = DataManager->GetUnit(1, 3, ETacGridLayer::Ground);

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

	AUnit* Retrieved = DataManager->GetUnit(2, 2, ETacGridLayer::Ground);
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

	TestNull("Negative coords should return null", DataManager->GetUnit(-1, 2, ETacGridLayer::Ground));
	TestNull("Out of bounds should return null", DataManager->GetUnit(10, 10, ETacGridLayer::Ground));

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

	DataManager->PlaceUnit(Unit, 2, 2, ETacGridLayer::Ground);
	bool bRemoved = DataManager->RemoveUnit(2, 2, ETacGridLayer::Ground);

	TestTrue("Remove should succeed", bRemoved);
	TestNull("Cell should be empty after removal", DataManager->GetUnit(2, 2, ETacGridLayer::Ground));

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

	bool bRemoved = DataManager->RemoveUnit(2, 2, ETacGridLayer::Ground);
	TestFalse("Remove from empty cell should fail", bRemoved);

	Grid->Destroy();
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

	DataManager->PlaceUnit(Unit, 2, 2, ETacGridLayer::Ground);
	DataManager->SetUnitFlankState(Unit, true);
	DataManager->SetUnitOriginalRotation(Unit, FRotator(0, 90, 0));

	DataManager->RemoveUnit(2, 2, ETacGridLayer::Ground);

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

	DataManager->PlaceUnit(Unit, 3, 1, ETacGridLayer::Air);

	FTacCoordinates OutPosition;
	ETacGridLayer Layer;
	bool bFound = DataManager->GetUnitPosition(Unit, OutPosition, Layer);

	TestTrue("Should find unit position", bFound);
	TestEqual("Row should match", OutPosition.Row, 3);
	TestEqual("Col should match", OutPosition.Col, 1);
	TestEqual("Layer should match", Layer, ETacGridLayer::Air);

	Grid->Destroy();
	Unit->Destroy();
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

	FTacCoordinates OutPosition;
	ETacGridLayer Layer;
	bool bFound = DataManager->GetUnitPosition(Unit, OutPosition, Layer);

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

	DataManager->PushCorpse(Corpse, FTacCoordinates(2, 2));

	TestTrue("Should have corpses", DataManager->HasCorpses(FTacCoordinates(2, 2)));
	TestEqual("Top corpse should match", DataManager->GetTopCorpse(FTacCoordinates(2, 2)), Corpse);

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

	DataManager->PushCorpse(Corpse1, FTacCoordinates(2, 2));
	DataManager->PushCorpse(Corpse2, FTacCoordinates(2, 2));
	DataManager->PushCorpse(Corpse3, FTacCoordinates(2, 2));

	TestEqual("Top corpse should be last pushed", DataManager->GetTopCorpse(FTacCoordinates(2, 2)), Corpse3);
	const TArray<TObjectPtr<AUnit>>& Stack = DataManager->GetCorpseStack(FTacCoordinates(2, 2));
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

	DataManager->PushCorpse(Corpse1, FTacCoordinates(2, 2));
	DataManager->PushCorpse(Corpse2, FTacCoordinates(2, 2));

	AUnit* Popped = DataManager->PopCorpse(FTacCoordinates(2, 2));
	TestEqual("Popped corpse should be Corpse2", Popped, Corpse2);
	TestEqual("New top should be Corpse1", DataManager->GetTopCorpse(FTacCoordinates(2, 2)), Corpse1);

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

	AUnit* Popped = DataManager->PopCorpse(FTacCoordinates(2, 2));
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

	DataManager->PushCorpse(Corpse, FTacCoordinates(2, 2));

	AUnit* Top1 = DataManager->GetTopCorpse(FTacCoordinates(2, 2));
	AUnit* Top2 = DataManager->GetTopCorpse(FTacCoordinates(2, 2));

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

	TestFalse("Empty cell has no corpses", DataManager->HasCorpses(FTacCoordinates(2, 2)));

	DataManager->PushCorpse(Corpse, FTacCoordinates(2, 2));
	TestTrue("Cell with corpse returns true", DataManager->HasCorpses(FTacCoordinates(2, 2)));

	Grid->Destroy();
	Corpse->Destroy();
	return true;
}

// Test: GetTeamBySide returns correct team object
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetTeamBySideTest,
	"KBS.Grid.Components.DataManager.GetTeamBySide_ReturnsCorrectTeam",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetTeamBySideTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);

	TestEqual("Attacker side returns attacker team", DataManager->GetTeamBySide(ETeamSide::Attacker), DataManager->GetAttackerTeam());
	TestEqual("Defender side returns defender team", DataManager->GetTeamBySide(ETeamSide::Defender), DataManager->GetDefenderTeam());

	Grid->Destroy();
	return true;
}

// Test: GetUnits(OnField) returns only on-field units
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridDataManagerGetUnitsOnFieldTest,
	"KBS.Grid.Components.DataManager.GetUnits_OnField_ReturnsOnFieldUnits",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridDataManagerGetUnitsOnFieldTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid = FGridDataManagerTestHelper::CreateMockGrid(World);
	UGridDataManager* DataManager = FGridDataManagerTestHelper::CreateDataManager(World, Grid);
	AUnit* Unit1 = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Attacker);
	AUnit* Unit2 = FGridDataManagerTestHelper::CreateMockUnit(World, ETeamSide::Attacker);

	DataManager->GetAttackerTeam()->AddUnit(Unit1);
	DataManager->GetAttackerTeam()->AddUnit(Unit2);
	DataManager->PlaceUnit(Unit1, FTacCoordinates(1, 1));
	DataManager->PlaceUnit(Unit2, FTacCoordinates(2, 1));

	TArray<AUnit*> OnField = DataManager->GetUnits(EUnitQuerySource::OnField);
	TestEqual("Should return 2 on-field units", OnField.Num(), 2);

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

	TArray<FTacCoordinates> EmptyBefore = DataManager->GetEmptyCells(ETacGridLayer::Ground);
	int32 EmptyCountBefore = EmptyBefore.Num();

	DataManager->PlaceUnit(Unit, 2, 2, ETacGridLayer::Ground);
	TArray<FTacCoordinates> EmptyAfter = DataManager->GetEmptyCells(ETacGridLayer::Ground);

	TestEqual("Empty cells should decrease by 1", EmptyAfter.Num(), EmptyCountBefore - 1);
	TestFalse("Occupied cell should not be in empty list", EmptyAfter.Contains(FTacCoordinates(2, 2)));

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
	DataManager->PlaceUnit(AttackerUnit, 1, 1, ETacGridLayer::Ground);
	DataManager->PlaceUnit(DefenderUnit, 3, 3, ETacGridLayer::Ground);

	TArray<FTacCoordinates> AttackerCells = DataManager->GetOccupiedCells(ETacGridLayer::Ground, DataManager->GetAttackerTeam());
	TestEqual("Should have 1 attacker cell", AttackerCells.Num(), 1);
	TestTrue("Should contain attacker cell", AttackerCells.Contains(FTacCoordinates(1, 1)));
	TestFalse("Should not contain defender cell", AttackerCells.Contains(FTacCoordinates(3, 3)));

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

	TestFalse("Empty cell not occupied", DataManager->IsCellOccupied(FTacCoordinates(2, 2, ETacGridLayer::Ground)));

	DataManager->PlaceUnit(Unit, 2, 2, ETacGridLayer::Ground);
	TestTrue("Occupied cell returns true", DataManager->IsCellOccupied(FTacCoordinates(2, 2, ETacGridLayer::Ground)));

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

	TArray<FTacCoordinates> ValidCells = DataManager->GetValidPlacementCells(ETacGridLayer::Ground);

	TestFalse("Restricted (2,0) should not be included", ValidCells.Contains(FTacCoordinates(2, 0)));
	TestFalse("Restricted (2,4) should not be included", ValidCells.Contains(FTacCoordinates(2, 4)));

	Grid->Destroy();
	return true;
}

