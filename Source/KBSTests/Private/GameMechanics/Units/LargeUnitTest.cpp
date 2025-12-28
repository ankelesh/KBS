#include "Misc/AutomationTest.h"
#include "GameMechanics/Units/LargeUnit.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/Components/GridMovementComponent.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/CombatTypes.h"

// Helper class for LargeUnit tests
class FLargeUnitTestHelper
{
public:
	static UUnitDefinition* CreateTestUnitDefinition(UObject* Outer)
	{
		UUnitDefinition* Definition = NewObject<UUnitDefinition>(Outer);
		Definition->UnitName = TEXT("TestLargeUnit");
		Definition->MovementSpeed = 300.0f;
		Definition->BaseStatsTemplate.MaxHealth = 200.0f;
		Definition->BaseStatsTemplate.Initiative = 40;
		Definition->BaseStatsTemplate.Accuracy = 0.7f;
		Definition->BaseStatsTemplate.Defense.DamageReduction = 10;
		return Definition;
	}

	static ALargeUnit* CreateMockLargeUnit(UWorld* World, float MaxHealth = 200.0f)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ALargeUnit* Unit = World->SpawnActor<ALargeUnit>(ALargeUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (Unit)
		{
			UUnitDefinition* Definition = CreateTestUnitDefinition(Unit);
			Definition->BaseStatsTemplate.MaxHealth = MaxHealth;
			Unit->SetUnitDefinition(Definition);
		}
		return Unit;
	}

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
};

//==============================================================================
// MODULE 1: BASIC PROPERTIES & SETUP
//==============================================================================

// Test 1: GetCellSize returns 2
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitGetCellSizeReturnsTwoTest,
	"KBS.Units.LargeUnit.GetCellSize_ReturnsTwo",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitGetCellSizeReturnsTwoTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	if (!LargeUnit)
	{
		AddError(TEXT("Failed to spawn LargeUnit actor - check ALargeUnit class registration"));
		return false;
	}

	int32 CellSize = LargeUnit->GetCellSize();
	if (CellSize != 2)
	{
		AddError(FString::Printf(TEXT("LargeUnit::GetCellSize() returned %d, expected 2 - large units should occupy 2 cells"), CellSize));
		LargeUnit->Destroy();
		return false;
	}

	LargeUnit->Destroy();
	return true;
}

// Test 2: IsMultiCell returns true
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitIsMultiCellReturnsTrueTest,
	"KBS.Units.LargeUnit.IsMultiCell_ReturnsTrue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitIsMultiCellReturnsTrueTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	if (!LargeUnit)
	{
		AddError(TEXT("Failed to spawn LargeUnit actor - check ALargeUnit class registration"));
		return false;
	}

	bool bIsMultiCell = LargeUnit->IsMultiCell();
	if (!bIsMultiCell)
	{
		AddError(TEXT("LargeUnit::IsMultiCell() returned false, expected true - large units should be marked as multi-cell"));
		LargeUnit->Destroy();
		return false;
	}

	LargeUnit->Destroy();
	return true;
}

// Test 3: Spawn succeeds
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitSpawnSucceedsTest,
	"KBS.Units.LargeUnit.Spawn_Succeeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitSpawnSucceedsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ALargeUnit* LargeUnit = World->SpawnActor<ALargeUnit>(ALargeUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (!LargeUnit)
	{
		AddError(TEXT("Failed to spawn ALargeUnit - check class is properly registered and available in editor"));
		return false;
	}

	if (!LargeUnit->IsValidLowLevel())
	{
		AddError(TEXT("Spawned LargeUnit is not valid - object may be corrupted or improperly initialized"));
		return false;
	}

	// Verify it's actually a LargeUnit and not just a base Unit
	AUnit* AsBaseUnit = Cast<AUnit>(LargeUnit);
	if (!AsBaseUnit)
	{
		AddError(TEXT("LargeUnit cannot be cast to AUnit - inheritance chain is broken"));
		LargeUnit->Destroy();
		return false;
	}

	ALargeUnit* AsLargeUnit = Cast<ALargeUnit>(AsBaseUnit);
	if (!AsLargeUnit)
	{
		AddError(TEXT("Unit cannot be cast back to ALargeUnit - class type mismatch"));
		LargeUnit->Destroy();
		return false;
	}

	LargeUnit->Destroy();
	return true;
}

//==============================================================================
// MODULE 2: SECONDARY CELL CALCULATION
//==============================================================================

// Test 4: GetSecondaryCell for Attacker vertical - Row+1
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitGetSecondaryCellAttackerVerticalTest,
	"KBS.Units.LargeUnit.GetSecondaryCell_AttackerVertical_RowPlusOne",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitGetSecondaryCellAttackerVerticalTest::RunTest(const FString& Parameters)
{
	int32 PrimaryRow = 2;
	int32 PrimaryCol = 2;
	bool bIsHorizontal = false;
	ETeamSide Team = ETeamSide::Attacker;

	FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(PrimaryRow, PrimaryCol, bIsHorizontal, Team);

	if (SecondaryCell.Y != PrimaryRow + 1)
	{
		AddError(FString::Printf(TEXT("Attacker vertical placement: secondary row is %d, expected %d (PrimaryRow+1) - attackers should extend forward"),
			SecondaryCell.Y, PrimaryRow + 1));
		return false;
	}

	if (SecondaryCell.X != PrimaryCol)
	{
		AddError(FString::Printf(TEXT("Attacker vertical placement: secondary col is %d, expected %d (same column) - vertical units share the same column"),
			SecondaryCell.X, PrimaryCol));
		return false;
	}

	return true;
}

// Test 5: GetSecondaryCell for Defender vertical - Row-1
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitGetSecondaryCellDefenderVerticalTest,
	"KBS.Units.LargeUnit.GetSecondaryCell_DefenderVertical_RowMinusOne",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitGetSecondaryCellDefenderVerticalTest::RunTest(const FString& Parameters)
{
	int32 PrimaryRow = 2;
	int32 PrimaryCol = 2;
	bool bIsHorizontal = false;
	ETeamSide Team = ETeamSide::Defender;

	FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(PrimaryRow, PrimaryCol, bIsHorizontal, Team);

	if (SecondaryCell.Y != PrimaryRow - 1)
	{
		AddError(FString::Printf(TEXT("Defender vertical placement: secondary row is %d, expected %d (PrimaryRow-1) - defenders should extend backward"),
			SecondaryCell.Y, PrimaryRow - 1));
		return false;
	}

	if (SecondaryCell.X != PrimaryCol)
	{
		AddError(FString::Printf(TEXT("Defender vertical placement: secondary col is %d, expected %d (same column) - vertical units share the same column"),
			SecondaryCell.X, PrimaryCol));
		return false;
	}

	return true;
}

// Test 6: GetSecondaryCell for horizontal left flank - Col+1
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitGetSecondaryCellHorizontalLeftFlankTest,
	"KBS.Units.LargeUnit.GetSecondaryCell_HorizontalLeftFlank_ColPlusOne",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitGetSecondaryCellHorizontalLeftFlankTest::RunTest(const FString& Parameters)
{
	int32 PrimaryRow = 2;
	int32 PrimaryCol = 0; // Left flank
	bool bIsHorizontal = true;
	ETeamSide Team = ETeamSide::Attacker; // Team doesn't matter for horizontal

	FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(PrimaryRow, PrimaryCol, bIsHorizontal, Team);

	if (SecondaryCell.X != PrimaryCol + 1)
	{
		AddError(FString::Printf(TEXT("Horizontal left flank placement: secondary col is %d, expected %d (PrimaryCol+1) - left flank should extend toward center"),
			SecondaryCell.X, PrimaryCol + 1));
		return false;
	}

	if (SecondaryCell.Y != PrimaryRow)
	{
		AddError(FString::Printf(TEXT("Horizontal left flank placement: secondary row is %d, expected %d (same row) - horizontal units share the same row"),
			SecondaryCell.Y, PrimaryRow));
		return false;
	}

	return true;
}

// Test 7: GetSecondaryCell for horizontal right flank - Col-1
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitGetSecondaryCellHorizontalRightFlankTest,
	"KBS.Units.LargeUnit.GetSecondaryCell_HorizontalRightFlank_ColMinusOne",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitGetSecondaryCellHorizontalRightFlankTest::RunTest(const FString& Parameters)
{
	int32 PrimaryRow = 2;
	int32 PrimaryCol = 4; // Right flank
	bool bIsHorizontal = true;
	ETeamSide Team = ETeamSide::Defender; // Team doesn't matter for horizontal

	FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(PrimaryRow, PrimaryCol, bIsHorizontal, Team);

	if (SecondaryCell.X != PrimaryCol - 1)
	{
		AddError(FString::Printf(TEXT("Horizontal right flank placement: secondary col is %d, expected %d (PrimaryCol-1) - right flank should extend toward center"),
			SecondaryCell.X, PrimaryCol - 1));
		return false;
	}

	if (SecondaryCell.Y != PrimaryRow)
	{
		AddError(FString::Printf(TEXT("Horizontal right flank placement: secondary row is %d, expected %d (same row) - horizontal units share the same row"),
			SecondaryCell.Y, PrimaryRow));
		return false;
	}

	return true;
}

//==============================================================================
// MODULE 3: PLACEMENT & POSITIONING
//==============================================================================

// Test 8: PlaceUnit occupies both cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitPlaceUnitBothCellsOccupiedTest,
	"KBS.Units.LargeUnit.PlaceUnit_ValidCells_BothCellsOccupied",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitPlaceUnitBothCellsOccupiedTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FLargeUnitTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	LargeUnit->SetTeamSide(ETeamSide::Attacker);

	int32 PrimaryRow = 1;
	int32 PrimaryCol = 2;
	bool bPlaced = DataManager->PlaceUnit(LargeUnit, PrimaryRow, PrimaryCol, EBattleLayer::Ground, Grid);

	if (!bPlaced)
	{
		AddError(FString::Printf(TEXT("Failed to place LargeUnit at (%d,%d) - placement should succeed on valid cells"), PrimaryRow, PrimaryCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	// Check primary cell
	AUnit* PrimaryUnit = DataManager->GetUnit(PrimaryRow, PrimaryCol, EBattleLayer::Ground);
	if (PrimaryUnit != LargeUnit)
	{
		AddError(FString::Printf(TEXT("Primary cell (%d,%d) does not contain the LargeUnit - expected unit pointer %p, got %p"),
			PrimaryRow, PrimaryCol, LargeUnit, PrimaryUnit));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	// Calculate secondary cell
	FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(PrimaryRow, PrimaryCol, false, ETeamSide::Attacker);

	// Check secondary cell
	AUnit* SecondaryUnit = DataManager->GetUnit(SecondaryCell.Y, SecondaryCell.X, EBattleLayer::Ground);
	if (SecondaryUnit != LargeUnit)
	{
		AddError(FString::Printf(TEXT("Secondary cell (%d,%d) does not contain the LargeUnit - expected unit pointer %p, got %p. Both cells should reference the same unit"),
			SecondaryCell.Y, SecondaryCell.X, LargeUnit, SecondaryUnit));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	Grid->Destroy();
	LargeUnit->Destroy();
	return true;
}

// Test 9: PlaceUnit fails if secondary cell is occupied
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitPlaceUnitSecondaryCellOccupiedFailsTest,
	"KBS.Units.LargeUnit.PlaceUnit_SecondaryCellOccupied_Fails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitPlaceUnitSecondaryCellOccupiedFailsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FLargeUnitTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	// Place a regular unit in what will be the secondary cell
	int32 PrimaryRow = 1;
	int32 PrimaryCol = 2;
	FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(PrimaryRow, PrimaryCol, false, ETeamSide::Attacker);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AUnit* BlockingUnit = World->SpawnActor<AUnit>(AUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	BlockingUnit->SetTeamSide(ETeamSide::Attacker);

	bool bBlockerPlaced = DataManager->PlaceUnit(BlockingUnit, SecondaryCell.Y, SecondaryCell.X, EBattleLayer::Ground, Grid);
	if (!bBlockerPlaced)
	{
		AddError(FString::Printf(TEXT("Failed to place blocking unit at secondary cell (%d,%d) - test setup failed"),
			SecondaryCell.Y, SecondaryCell.X));
		Grid->Destroy();
		BlockingUnit->Destroy();
		return false;
	}

	// Try to place LargeUnit
	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	LargeUnit->SetTeamSide(ETeamSide::Attacker);

	bool bPlaced = DataManager->PlaceUnit(LargeUnit, PrimaryRow, PrimaryCol, EBattleLayer::Ground, Grid);

	if (bPlaced)
	{
		AddError(FString::Printf(TEXT("LargeUnit placement succeeded at (%d,%d) when secondary cell (%d,%d) was occupied - should fail"),
			PrimaryRow, PrimaryCol, SecondaryCell.Y, SecondaryCell.X));
		Grid->Destroy();
		LargeUnit->Destroy();
		BlockingUnit->Destroy();
		return false;
	}

	Grid->Destroy();
	LargeUnit->Destroy();
	BlockingUnit->Destroy();
	return true;
}

// Test 10: PlaceUnit fails if secondary cell is out of bounds
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitPlaceUnitSecondaryCellOutOfBoundsFailsTest,
	"KBS.Units.LargeUnit.PlaceUnit_SecondaryCellOutOfBounds_Fails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitPlaceUnitSecondaryCellOutOfBoundsFailsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FLargeUnitTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	LargeUnit->SetTeamSide(ETeamSide::Attacker);

	// Try to place at edge where secondary would be out of bounds
	// For Attacker vertical, placing at row 4 would need secondary at row 5 (out of bounds)
	int32 PrimaryRow = 4;
	int32 PrimaryCol = 2;

	bool bPlaced = DataManager->PlaceUnit(LargeUnit, PrimaryRow, PrimaryCol, EBattleLayer::Ground, Grid);

	if (bPlaced)
	{
		FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(PrimaryRow, PrimaryCol, false, ETeamSide::Attacker);
		AddError(FString::Printf(TEXT("LargeUnit placement succeeded at (%d,%d) when secondary cell would be at (%d,%d) which is out of bounds - should fail"),
			PrimaryRow, PrimaryCol, SecondaryCell.Y, SecondaryCell.X));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	Grid->Destroy();
	LargeUnit->Destroy();
	return true;
}

// Test 11: PlaceUnit on flank cells succeeds
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitPlaceUnitFlankCellsSucceedsTest,
	"KBS.Units.LargeUnit.PlaceUnit_FlankCells_Succeeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitPlaceUnitFlankCellsSucceedsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FLargeUnitTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	// Test left flank (col 0) - use row 1 (not center row 2 which is restricted)
	ALargeUnit* LeftFlankUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	LeftFlankUnit->SetTeamSide(ETeamSide::Attacker);

	int32 LeftFlankRow = 1;
	int32 LeftFlankCol = 0; // Left flank

	bool bLeftPlaced = DataManager->PlaceUnit(LeftFlankUnit, LeftFlankRow, LeftFlankCol, EBattleLayer::Ground, Grid);

	if (!bLeftPlaced)
	{
		AddError(FString::Printf(TEXT("Failed to place LargeUnit on left flank at (%d,%d) - flank placement should be allowed"),
			LeftFlankRow, LeftFlankCol));
		Grid->Destroy();
		LeftFlankUnit->Destroy();
		return false;
	}

	// Test right flank (col 4) - use row 3 (not center row 2 which is restricted)
	ALargeUnit* RightFlankUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	RightFlankUnit->SetTeamSide(ETeamSide::Defender);

	int32 RightFlankRow = 3;
	int32 RightFlankCol = 4; // Right flank

	bool bRightPlaced = DataManager->PlaceUnit(RightFlankUnit, RightFlankRow, RightFlankCol, EBattleLayer::Ground, Grid);

	if (!bRightPlaced)
	{
		AddError(FString::Printf(TEXT("Failed to place LargeUnit on right flank at (%d,%d) - flank placement should be allowed"),
			RightFlankRow, RightFlankCol));
		Grid->Destroy();
		LeftFlankUnit->Destroy();
		RightFlankUnit->Destroy();
		return false;
	}

	Grid->Destroy();
	LeftFlankUnit->Destroy();
	RightFlankUnit->Destroy();
	return true;
}

// Test 12: PlaceUnit on air layer succeeds when both cells free
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitPlaceUnitAirLayerSucceedsTest,
	"KBS.Units.LargeUnit.PlaceUnit_AirLayer_BothCellsFree_Succeeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitPlaceUnitAirLayerSucceedsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FLargeUnitTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	LargeUnit->SetTeamSide(ETeamSide::Attacker);

	int32 PrimaryRow = 1;
	int32 PrimaryCol = 2;
	bool bPlaced = DataManager->PlaceUnit(LargeUnit, PrimaryRow, PrimaryCol, EBattleLayer::Air, Grid);

	if (!bPlaced)
	{
		AddError(FString::Printf(TEXT("Failed to place LargeUnit on air layer at (%d,%d) - air layer placement should succeed when cells are free"),
			PrimaryRow, PrimaryCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	// Verify both cells are occupied on air layer
	AUnit* PrimaryUnit = DataManager->GetUnit(PrimaryRow, PrimaryCol, EBattleLayer::Air);
	if (PrimaryUnit != LargeUnit)
	{
		AddError(FString::Printf(TEXT("Air layer primary cell (%d,%d) does not contain LargeUnit after placement"),
			PrimaryRow, PrimaryCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(PrimaryRow, PrimaryCol, false, ETeamSide::Attacker);
	AUnit* SecondaryUnit = DataManager->GetUnit(SecondaryCell.Y, SecondaryCell.X, EBattleLayer::Air);
	if (SecondaryUnit != LargeUnit)
	{
		AddError(FString::Printf(TEXT("Air layer secondary cell (%d,%d) does not contain LargeUnit after placement - both cells should be occupied"),
			SecondaryCell.Y, SecondaryCell.X));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	Grid->Destroy();
	LargeUnit->Destroy();
	return true;
}

// Test 13: PlaceUnit in center cells succeeds
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitPlaceUnitCenterCellsSucceedsTest,
	"KBS.Units.LargeUnit.PlaceUnit_CenterCells_Succeeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitPlaceUnitCenterCellsSucceedsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FLargeUnitTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	LargeUnit->SetTeamSide(ETeamSide::Attacker);

	// Place in center area (non-flank cells)
	int32 PrimaryRow = 1;
	int32 PrimaryCol = 2; // Center column

	bool bPlaced = DataManager->PlaceUnit(LargeUnit, PrimaryRow, PrimaryCol, EBattleLayer::Ground, Grid);

	if (!bPlaced)
	{
		AddError(FString::Printf(TEXT("Failed to place LargeUnit in center cells at (%d,%d) - center placement should succeed"),
			PrimaryRow, PrimaryCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	// Verify placement
	int32 OutRow, OutCol;
	EBattleLayer OutLayer;
	bool bFound = DataManager->GetUnitPosition(LargeUnit, OutRow, OutCol, OutLayer);

	if (!bFound)
	{
		AddError(TEXT("LargeUnit not found in grid after placement - DataManager lost track of unit"));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	if (OutRow != PrimaryRow || OutCol != PrimaryCol)
	{
		AddError(FString::Printf(TEXT("LargeUnit position is (%d,%d), expected (%d,%d) - position mismatch after placement"),
			OutRow, OutCol, PrimaryRow, PrimaryCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	Grid->Destroy();
	LargeUnit->Destroy();
	return true;
}

//==============================================================================
// MODULE 4: MOVEMENT
//==============================================================================

// Test 14: Move unit vertically when both cells free succeeds
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitMoveVerticalBothCellsFreeSucceedsTest,
	"KBS.Units.LargeUnit.MoveUnit_VerticalBothCellsFree_Succeeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitMoveVerticalBothCellsFreeSucceedsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FLargeUnitTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	LargeUnit->SetTeamSide(ETeamSide::Attacker);

	// Place unit at (1,2)
	int32 StartRow = 1;
	int32 StartCol = 2;
	DataManager->PlaceUnit(LargeUnit, StartRow, StartCol, EBattleLayer::Ground, Grid);

	// Move vertically (within same column)
	int32 TargetRow = 0;
	int32 TargetCol = 2;
	bool bMoved = Movement->MoveUnit(LargeUnit, TargetRow, TargetCol);

	if (!bMoved)
	{
		AddError(FString::Printf(TEXT("Vertical movement failed from (%d,%d) to (%d,%d) when both target cells were free - should succeed"),
			StartRow, StartCol, TargetRow, TargetCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	// Verify new position
	int32 OutRow, OutCol;
	EBattleLayer OutLayer;
	DataManager->GetUnitPosition(LargeUnit, OutRow, OutCol, OutLayer);

	if (OutRow != TargetRow || OutCol != TargetCol)
	{
		AddError(FString::Printf(TEXT("After vertical move, unit is at (%d,%d), expected (%d,%d) - position not updated correctly"),
			OutRow, OutCol, TargetRow, TargetCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	Grid->Destroy();
	LargeUnit->Destroy();
	return true;
}

// Test 15: Move unit horizontally when both cells free succeeds
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitMoveHorizontalBothCellsFreeSucceedsTest,
	"KBS.Units.LargeUnit.MoveUnit_HorizontalBothCellsFree_Succeeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitMoveHorizontalBothCellsFreeSucceedsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FLargeUnitTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	LargeUnit->SetTeamSide(ETeamSide::Attacker);

	// Place unit at (1,2)
	int32 StartRow = 1;
	int32 StartCol = 2;
	DataManager->PlaceUnit(LargeUnit, StartRow, StartCol, EBattleLayer::Ground, Grid);

	// Move horizontally (within same row)
	int32 TargetRow = 1;
	int32 TargetCol = 3;
	bool bMoved = Movement->MoveUnit(LargeUnit, TargetRow, TargetCol);

	if (!bMoved)
	{
		AddError(FString::Printf(TEXT("Horizontal movement failed from (%d,%d) to (%d,%d) when both target cells were free - should succeed"),
			StartRow, StartCol, TargetRow, TargetCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	// Verify new position
	int32 OutRow, OutCol;
	EBattleLayer OutLayer;
	DataManager->GetUnitPosition(LargeUnit, OutRow, OutCol, OutLayer);

	if (OutRow != TargetRow || OutCol != TargetCol)
	{
		AddError(FString::Printf(TEXT("After horizontal move, unit is at (%d,%d), expected (%d,%d) - position not updated correctly"),
			OutRow, OutCol, TargetRow, TargetCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	Grid->Destroy();
	LargeUnit->Destroy();
	return true;
}

// Test 16: Movement fails if target cell is occupied
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitMoveTargetCellOccupiedFailsTest,
	"KBS.Units.LargeUnit.MoveUnit_TargetCellOccupied_Fails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitMoveTargetCellOccupiedFailsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FLargeUnitTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	LargeUnit->SetTeamSide(ETeamSide::Attacker);

	// Place LargeUnit at (1,2)
	int32 StartRow = 1;
	int32 StartCol = 2;
	DataManager->PlaceUnit(LargeUnit, StartRow, StartCol, EBattleLayer::Ground, Grid);

	// Place blocking unit at target location
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AUnit* BlockingUnit = World->SpawnActor<AUnit>(AUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	BlockingUnit->SetTeamSide(ETeamSide::Attacker);

	int32 TargetRow = 0;
	int32 TargetCol = 2;
	DataManager->PlaceUnit(BlockingUnit, TargetRow, TargetCol, EBattleLayer::Ground, Grid);

	// Try to move LargeUnit to occupied cell
	bool bMoved = Movement->MoveUnit(LargeUnit, TargetRow, TargetCol);

	if (bMoved)
	{
		AddError(FString::Printf(TEXT("LargeUnit movement succeeded from (%d,%d) to occupied cell (%d,%d) - should fail when target is blocked"),
			StartRow, StartCol, TargetRow, TargetCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		BlockingUnit->Destroy();
		return false;
	}

	// Verify LargeUnit didn't move
	int32 OutRow, OutCol;
	EBattleLayer OutLayer;
	DataManager->GetUnitPosition(LargeUnit, OutRow, OutCol, OutLayer);

	if (OutRow != StartRow || OutCol != StartCol)
	{
		AddError(FString::Printf(TEXT("LargeUnit position changed to (%d,%d) after failed move, should remain at (%d,%d)"),
			OutRow, OutCol, StartRow, StartCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		BlockingUnit->Destroy();
		return false;
	}

	Grid->Destroy();
	LargeUnit->Destroy();
	BlockingUnit->Destroy();
	return true;
}

// Test 17: LargeUnit cannot swap with 1-cell unit
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitCannotSwapWith1CellUnitTest,
	"KBS.Units.LargeUnit.MoveUnit_SwapWith1CellUnit_Fails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitCannotSwapWith1CellUnitTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FLargeUnitTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	LargeUnit->SetTeamSide(ETeamSide::Attacker);

	// Place LargeUnit at (1,2)
	int32 LargeUnitRow = 1;
	int32 LargeUnitCol = 2;
	DataManager->PlaceUnit(LargeUnit, LargeUnitRow, LargeUnitCol, EBattleLayer::Ground, Grid);

	// Place 1-cell unit adjacent
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AUnit* SmallUnit = World->SpawnActor<AUnit>(AUnit::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	SmallUnit->SetTeamSide(ETeamSide::Attacker);

	int32 SmallUnitRow = 1;
	int32 SmallUnitCol = 3;
	DataManager->PlaceUnit(SmallUnit, SmallUnitRow, SmallUnitCol, EBattleLayer::Ground, Grid);

	// Try to move LargeUnit to SmallUnit's position (would attempt swap)
	bool bMoved = Movement->MoveUnit(LargeUnit, SmallUnitRow, SmallUnitCol);

	if (bMoved)
	{
		AddError(FString::Printf(TEXT("LargeUnit swap with 1-cell unit succeeded from (%d,%d) to (%d,%d) - large units cannot swap with regular units"),
			LargeUnitRow, LargeUnitCol, SmallUnitRow, SmallUnitCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		SmallUnit->Destroy();
		return false;
	}

	// Verify neither unit moved
	int32 OutRow, OutCol;
	EBattleLayer OutLayer;

	DataManager->GetUnitPosition(LargeUnit, OutRow, OutCol, OutLayer);
	if (OutRow != LargeUnitRow || OutCol != LargeUnitCol)
	{
		AddError(FString::Printf(TEXT("LargeUnit moved from (%d,%d) to (%d,%d) during failed swap - should stay in place"),
			LargeUnitRow, LargeUnitCol, OutRow, OutCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		SmallUnit->Destroy();
		return false;
	}

	DataManager->GetUnitPosition(SmallUnit, OutRow, OutCol, OutLayer);
	if (OutRow != SmallUnitRow || OutCol != SmallUnitCol)
	{
		AddError(FString::Printf(TEXT("Small unit moved from (%d,%d) to (%d,%d) during failed swap - should stay in place"),
			SmallUnitRow, SmallUnitCol, OutRow, OutCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		SmallUnit->Destroy();
		return false;
	}

	Grid->Destroy();
	LargeUnit->Destroy();
	SmallUnit->Destroy();
	return true;
}

// Test 18: Successful move updates both old and new cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitMoveUpdatesBothCellsTest,
	"KBS.Units.LargeUnit.MoveUnit_Success_UpdatesBothCells",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitMoveUpdatesBothCellsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FLargeUnitTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	LargeUnit->SetTeamSide(ETeamSide::Attacker);

	// Place unit at (1,2) - secondary will be at (2,2)
	int32 StartRow = 1;
	int32 StartCol = 2;
	DataManager->PlaceUnit(LargeUnit, StartRow, StartCol, EBattleLayer::Ground, Grid);

	FIntPoint OldSecondary = ALargeUnit::GetSecondaryCell(StartRow, StartCol, false, ETeamSide::Attacker);

	// Move to (0,2) - secondary will be at (1,2)
	int32 TargetRow = 0;
	int32 TargetCol = 2;
	FIntPoint NewSecondary = ALargeUnit::GetSecondaryCell(TargetRow, TargetCol, false, ETeamSide::Attacker);

	bool bMoved = Movement->MoveUnit(LargeUnit, TargetRow, TargetCol);

	if (!bMoved)
	{
		AddError(FString::Printf(TEXT("Failed to move LargeUnit from (%d,%d) to (%d,%d) - movement should succeed"),
			StartRow, StartCol, TargetRow, TargetCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	// Check new primary cell contains unit
	AUnit* NewPrimaryUnit = DataManager->GetUnit(TargetRow, TargetCol, EBattleLayer::Ground);
	if (NewPrimaryUnit != LargeUnit)
	{
		AddError(FString::Printf(TEXT("New primary cell (%d,%d) contains unit pointer %p, expected %p - should contain the moved unit"),
			TargetRow, TargetCol, NewPrimaryUnit, LargeUnit));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	// Check new secondary cell contains unit
	AUnit* NewSecondaryUnit = DataManager->GetUnit(NewSecondary.Y, NewSecondary.X, EBattleLayer::Ground);
	if (NewSecondaryUnit != LargeUnit)
	{
		AddError(FString::Printf(TEXT("New secondary cell (%d,%d) contains unit pointer %p, expected %p - should contain the moved unit"),
			NewSecondary.Y, NewSecondary.X, NewSecondaryUnit, LargeUnit));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	// Check old cells that are NOT part of new position are empty
	// Old primary (1,2) may overlap with new secondary (1,2) - only check if distinct
	bool bOldPrimaryIsNewCell = (StartRow == TargetRow && StartCol == TargetCol) ||
	                             (StartRow == NewSecondary.Y && StartCol == NewSecondary.X);
	if (!bOldPrimaryIsNewCell)
	{
		AUnit* OldPrimaryUnit = DataManager->GetUnit(StartRow, StartCol, EBattleLayer::Ground);
		if (OldPrimaryUnit != nullptr)
		{
			AddError(FString::Printf(TEXT("Old primary cell (%d,%d) still contains unit pointer %p after move - should be empty"),
				StartRow, StartCol, OldPrimaryUnit));
			Grid->Destroy();
			LargeUnit->Destroy();
			return false;
		}
	}

	// Old secondary (2,2) may overlap with new cells - only check if distinct
	bool bOldSecondaryIsNewCell = (OldSecondary.Y == TargetRow && OldSecondary.X == TargetCol) ||
	                               (OldSecondary.Y == NewSecondary.Y && OldSecondary.X == NewSecondary.X);
	if (!bOldSecondaryIsNewCell)
	{
		AUnit* OldSecondaryUnit = DataManager->GetUnit(OldSecondary.Y, OldSecondary.X, EBattleLayer::Ground);
		if (OldSecondaryUnit != nullptr)
		{
			AddError(FString::Printf(TEXT("Old secondary cell (%d,%d) still contains unit pointer %p after move - should be empty"),
				OldSecondary.Y, OldSecondary.X, OldSecondaryUnit));
			Grid->Destroy();
			LargeUnit->Destroy();
			return false;
		}
	}

	Grid->Destroy();
	LargeUnit->Destroy();
	return true;
}

//==============================================================================
// MODULE 5: COMBAT & DAMAGE
//==============================================================================

// Test 19: GetUnit on primary cell returns LargeUnit
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitGetUnitPrimaryCellReturnsLargeUnitTest,
	"KBS.Units.LargeUnit.GetUnit_PrimaryCell_ReturnsLargeUnit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitGetUnitPrimaryCellReturnsLargeUnitTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FLargeUnitTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	LargeUnit->SetTeamSide(ETeamSide::Attacker);

	int32 PrimaryRow = 1;
	int32 PrimaryCol = 2;
	DataManager->PlaceUnit(LargeUnit, PrimaryRow, PrimaryCol, EBattleLayer::Ground, Grid);

	// Query primary cell
	AUnit* RetrievedUnit = DataManager->GetUnit(PrimaryRow, PrimaryCol, EBattleLayer::Ground);

	if (RetrievedUnit != LargeUnit)
	{
		AddError(FString::Printf(TEXT("GetUnit on primary cell (%d,%d) returned %p, expected %p - primary cell should return the large unit"),
			PrimaryRow, PrimaryCol, RetrievedUnit, LargeUnit));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	// Verify it's actually a LargeUnit
	ALargeUnit* AsLargeUnit = Cast<ALargeUnit>(RetrievedUnit);
	if (!AsLargeUnit)
	{
		AddError(FString::Printf(TEXT("Retrieved unit from primary cell (%d,%d) is not a LargeUnit - type mismatch"),
			PrimaryRow, PrimaryCol));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	Grid->Destroy();
	LargeUnit->Destroy();
	return true;
}

// Test 20: GetUnit on secondary cell returns LargeUnit
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitGetUnitSecondaryCellReturnsLargeUnitTest,
	"KBS.Units.LargeUnit.GetUnit_SecondaryCell_ReturnsLargeUnit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitGetUnitSecondaryCellReturnsLargeUnitTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FLargeUnitTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World);
	LargeUnit->SetTeamSide(ETeamSide::Attacker);

	int32 PrimaryRow = 1;
	int32 PrimaryCol = 2;
	DataManager->PlaceUnit(LargeUnit, PrimaryRow, PrimaryCol, EBattleLayer::Ground, Grid);

	// Calculate and query secondary cell
	FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(PrimaryRow, PrimaryCol, false, ETeamSide::Attacker);
	AUnit* RetrievedUnit = DataManager->GetUnit(SecondaryCell.Y, SecondaryCell.X, EBattleLayer::Ground);

	if (RetrievedUnit != LargeUnit)
	{
		AddError(FString::Printf(TEXT("GetUnit on secondary cell (%d,%d) returned %p, expected %p - secondary cell should return the same large unit"),
			SecondaryCell.Y, SecondaryCell.X, RetrievedUnit, LargeUnit));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	// Verify it's actually a LargeUnit
	ALargeUnit* AsLargeUnit = Cast<ALargeUnit>(RetrievedUnit);
	if (!AsLargeUnit)
	{
		AddError(FString::Printf(TEXT("Retrieved unit from secondary cell (%d,%d) is not a LargeUnit - type mismatch"),
			SecondaryCell.Y, SecondaryCell.X));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	Grid->Destroy();
	LargeUnit->Destroy();
	return true;
}

// Test 21: TakeHit on primary cell reduces health
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitTakeHitOnPrimaryCellReducesHealthTest,
	"KBS.Units.LargeUnit.TakeHit_OnPrimaryCell_ReducesHealth",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitTakeHitOnPrimaryCellReducesHealthTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FLargeUnitTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World, 200.0f);
	LargeUnit->SetTeamSide(ETeamSide::Attacker);

	int32 PrimaryRow = 1;
	int32 PrimaryCol = 2;
	DataManager->PlaceUnit(LargeUnit, PrimaryRow, PrimaryCol, EBattleLayer::Ground, Grid);

	float InitialHealth = LargeUnit->GetCurrentHealth();
	if (InitialHealth != 200.0f)
	{
		AddError(FString::Printf(TEXT("Initial health is %.1f, expected 200.0 - test setup failed"), InitialHealth));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	// Hit the unit on primary cell
	FDamageResult DamageResult;
	DamageResult.Damage = 50;
	DamageResult.DamageSource = EDamageSource::Physical;
	LargeUnit->TakeHit(DamageResult);

	float CurrentHealth = LargeUnit->GetCurrentHealth();
	float ExpectedHealth = 150.0f;

	if (CurrentHealth != ExpectedHealth)
	{
		AddError(FString::Printf(TEXT("After taking 50 damage on primary cell, health is %.1f, expected %.1f - damage not applied correctly"),
			CurrentHealth, ExpectedHealth));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	Grid->Destroy();
	LargeUnit->Destroy();
	return true;
}

// Test 22: TakeHit on secondary cell reduces health
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeUnitTakeHitOnSecondaryCellReducesHealthTest,
	"KBS.Units.LargeUnit.TakeHit_OnSecondaryCell_ReducesHealth",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeUnitTakeHitOnSecondaryCellReducesHealthTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddError(TEXT("Failed to get editor world context - cannot run test"));
		return false;
	}

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridMovementComponent* Movement;
	FLargeUnitTestHelper::SetupGridWithComponents(World, Grid, DataManager, Movement);

	ALargeUnit* LargeUnit = FLargeUnitTestHelper::CreateMockLargeUnit(World, 200.0f);
	LargeUnit->SetTeamSide(ETeamSide::Attacker);

	int32 PrimaryRow = 1;
	int32 PrimaryCol = 2;
	DataManager->PlaceUnit(LargeUnit, PrimaryRow, PrimaryCol, EBattleLayer::Ground, Grid);

	float InitialHealth = LargeUnit->GetCurrentHealth();
	if (InitialHealth != 200.0f)
	{
		AddError(FString::Printf(TEXT("Initial health is %.1f, expected 200.0 - test setup failed"), InitialHealth));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	// Get secondary cell and verify the same unit is there
	FIntPoint SecondaryCell = ALargeUnit::GetSecondaryCell(PrimaryRow, PrimaryCol, false, ETeamSide::Attacker);
	AUnit* SecondaryUnit = DataManager->GetUnit(SecondaryCell.Y, SecondaryCell.X, EBattleLayer::Ground);

	if (SecondaryUnit != LargeUnit)
	{
		AddError(FString::Printf(TEXT("Secondary cell (%d,%d) does not contain the LargeUnit - test setup failed"),
			SecondaryCell.Y, SecondaryCell.X));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	// Hit the unit (hits affect the whole unit regardless of which cell was targeted)
	FDamageResult DamageResult;
	DamageResult.Damage = 50;
	DamageResult.DamageSource = EDamageSource::Physical;
	LargeUnit->TakeHit(DamageResult);

	float CurrentHealth = LargeUnit->GetCurrentHealth();
	float ExpectedHealth = 150.0f;

	if (CurrentHealth != ExpectedHealth)
	{
		AddError(FString::Printf(TEXT("After taking 50 damage on secondary cell, health is %.1f, expected %.1f - damage not applied correctly. Both cells represent the same unit."),
			CurrentHealth, ExpectedHealth));
		Grid->Destroy();
		LargeUnit->Destroy();
		return false;
	}

	Grid->Destroy();
	LargeUnit->Destroy();
	return true;
}
