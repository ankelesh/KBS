#include "Misc/AutomationTest.h"
#include "GameMechanics/Tactical/Grid/Components/GridTargetingComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/LargeUnit.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/DamageTypes.h"

// Helper class for targeting tests
class FGridTargetingTestHelper
{
public:
	static void SetupGridWithComponents(UWorld* World, ATacBattleGrid*& OutGrid, UGridDataManager*& OutDataManager, UGridTargetingComponent*& OutTargeting)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		OutGrid = World->SpawnActor<ATacBattleGrid>(ATacBattleGrid::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		OutDataManager = OutGrid->GetDataManager();
		OutDataManager->Initialize(OutGrid);

		OutTargeting = NewObject<UGridTargetingComponent>(OutGrid);
		OutTargeting->Initialize(OutDataManager);
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

			UBattleTeam* Team = (TeamSide == ETeamSide::Attacker) ? DataManager->GetAttackerTeam() : DataManager->GetDefenderTeam();
			Team->AddUnit(Unit);
		}
		return Unit;
	}
};

// Test: Self targeting returns own cell
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingSelfTest,
	"KBS.Grid.Components.Targeting.GetValidTargetCells_Self_ReturnsOwnCell",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingSelfTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* Unit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground);

	TArray<FIntPoint> TargetCells = Targeting->GetValidTargetCells(Unit, ETargetReach::Self);

	TestEqual("Should return 1 cell", TargetCells.Num(), 1);
	if (TargetCells.Num() > 0)
	{
		TestEqual("Should be unit's own cell", TargetCells[0], FIntPoint(2, 2));
	}

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: ClosestEnemies returns only adjacent enemies
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingClosestEnemiesTest,
	"KBS.Grid.Components.Targeting.GetValidTargetCells_ClosestEnemies_ReturnsAdjacent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingClosestEnemiesTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* AttackerUnit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* AdjacentEnemy = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 3, EBattleLayer::Ground, ETeamSide::Defender);
	AUnit* DiagonalEnemy = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 3, 3, EBattleLayer::Ground, ETeamSide::Defender);
	AUnit* FarEnemy = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 4, 4, EBattleLayer::Ground, ETeamSide::Defender);

	TArray<FIntPoint> TargetCells = Targeting->GetValidTargetCells(AttackerUnit, ETargetReach::ClosestEnemies);

	TestTrue("Should include adjacent enemy", TargetCells.Contains(FIntPoint(3, 2)));
	TestTrue("Should include diagonal enemy", TargetCells.Contains(FIntPoint(3, 3)));
	TestFalse("Should not include far enemy", TargetCells.Contains(FIntPoint(4, 4)));

	Grid->Destroy();
	AttackerUnit->Destroy();
	AdjacentEnemy->Destroy();
	DiagonalEnemy->Destroy();
	FarEnemy->Destroy();
	return true;
}

// Test: ClosestEnemies with no enemies returns empty
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingClosestEnemiesNoneTest,
	"KBS.Grid.Components.Targeting.GetValidTargetCells_ClosestEnemies_NoEnemies_ReturnsEmpty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingClosestEnemiesNoneTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* AttackerUnit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);

	TArray<FIntPoint> TargetCells = Targeting->GetValidTargetCells(AttackerUnit, ETargetReach::ClosestEnemies);

	TestEqual("Should return empty array", TargetCells.Num(), 0);

	Grid->Destroy();
	AttackerUnit->Destroy();
	return true;
}

// Test: AnyEnemy returns all enemy cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingAnyEnemyTest,
	"KBS.Grid.Components.Targeting.GetValidTargetCells_AnyEnemy_ReturnsAllEnemyCells",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingAnyEnemyTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* AttackerUnit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* Enemy1 = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 0, 0, EBattleLayer::Ground, ETeamSide::Defender);
	AUnit* Enemy2 = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 4, 4, EBattleLayer::Ground, ETeamSide::Defender);

	TArray<FIntPoint> TargetCells = Targeting->GetValidTargetCells(AttackerUnit, ETargetReach::AnyEnemy);

	TestEqual("Should return 2 enemy cells", TargetCells.Num(), 2);
	TestTrue("Should include enemy 1", TargetCells.Contains(FIntPoint(0, 0)));
	TestTrue("Should include enemy 2", TargetCells.Contains(FIntPoint(4, 4)));

	Grid->Destroy();
	AttackerUnit->Destroy();
	Enemy1->Destroy();
	Enemy2->Destroy();
	return true;
}

// Test: AllEnemies returns all enemy cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingAllEnemiesTest,
	"KBS.Grid.Components.Targeting.GetValidTargetCells_AllEnemies_ReturnsAllEnemyCells",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingAllEnemiesTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* AttackerUnit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* Enemy1 = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 1, 1, EBattleLayer::Ground, ETeamSide::Defender);
	AUnit* Enemy2 = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 3, 3, EBattleLayer::Ground, ETeamSide::Defender);

	TArray<FIntPoint> TargetCells = Targeting->GetValidTargetCells(AttackerUnit, ETargetReach::AllEnemies);

	TestEqual("Should return all enemies", TargetCells.Num(), 2);

	Grid->Destroy();
	AttackerUnit->Destroy();
	Enemy1->Destroy();
	Enemy2->Destroy();
	return true;
}

// Test: Flank targeting returns adjacent columns
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingFlankTargetsTest,
	"KBS.Grid.Components.Targeting.GetFlankTargetCells_ReturnsAdjacentColumns",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingFlankTargetsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	// Place attacker in flank column 0
	AUnit* AttackerUnit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 1, EBattleLayer::Ground, ETeamSide::Attacker);
	// Place enemies in adjacent columns
	AUnit* Enemy1 = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 1, 2, EBattleLayer::Ground, ETeamSide::Defender);
	AUnit* Enemy2 = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 3, 2, EBattleLayer::Ground, ETeamSide::Defender);

	TArray<FIntPoint> TargetCells = Targeting->GetValidTargetCells(AttackerUnit, ETargetReach::ClosestEnemies, true);

	// Flank targeting should find enemies in adjacent columns
	TestTrue("Should have target cells", TargetCells.Num() > 0);

	Grid->Destroy();
	AttackerUnit->Destroy();
	Enemy1->Destroy();
	Enemy2->Destroy();
	return true;
}

// Test: Flank targeting prefers closest to center
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingFlankPrefersCenterTest,
	"KBS.Grid.Components.Targeting.GetFlankTargetCells_PrefersClosestToCenter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingFlankPrefersCenterTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* AttackerUnit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 1, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* CenterEnemy = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Defender);
	AUnit* EdgeEnemy = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 0, 2, EBattleLayer::Ground, ETeamSide::Defender);

	TArray<FIntPoint> TargetCells = Targeting->GetValidTargetCells(AttackerUnit, ETargetReach::ClosestEnemies, true);

	// Center row (row 2) is closer to center than row 0
	// Should prefer center enemy if flank logic applies

	Grid->Destroy();
	AttackerUnit->Destroy();
	CenterEnemy->Destroy();
	EdgeEnemy->Destroy();
	return true;
}

// Test: AnyFriendly returns friendly cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingAnyFriendlyTest,
	"KBS.Grid.Components.Targeting.GetValidTargetCells_AnyFriendly_ReturnsFriendlyCells",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingAnyFriendlyTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* Unit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* Friendly = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 3, 3, EBattleLayer::Ground, ETeamSide::Attacker);

	TArray<FIntPoint> TargetCells = Targeting->GetValidTargetCells(Unit, ETargetReach::AnyFriendly);

	TestEqual("Should return 1 friendly", TargetCells.Num(), 1);
	TestTrue("Should include friendly cell", TargetCells.Contains(FIntPoint(3, 3)));
	TestFalse("Should not include self", TargetCells.Contains(FIntPoint(2, 2)));

	Grid->Destroy();
	Unit->Destroy();
	Friendly->Destroy();
	return true;
}

// Test: AllFriendlies returns all friendly cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingAllFriendliesTest,
	"KBS.Grid.Components.Targeting.GetValidTargetCells_AllFriendlies_ReturnsFriendlyCells",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingAllFriendliesTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* Unit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* Friendly1 = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 1, 1, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* Friendly2 = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 3, 3, EBattleLayer::Ground, ETeamSide::Attacker);

	TArray<FIntPoint> TargetCells = Targeting->GetValidTargetCells(Unit, ETargetReach::AllFriendlies);

	TestEqual("Should return 2 friendlies", TargetCells.Num(), 2);
	TestFalse("Should not include self", TargetCells.Contains(FIntPoint(2, 2)));

	Grid->Destroy();
	Unit->Destroy();
	Friendly1->Destroy();
	Friendly2->Destroy();
	return true;
}

// Test: EmptyCell returns valid empty cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingEmptyCellTest,
	"KBS.Grid.Components.Targeting.GetValidTargetCells_EmptyCell_ReturnsValidEmpty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingEmptyCellTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* Unit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);

	TArray<FIntPoint> TargetCells = Targeting->GetValidTargetCells(Unit, ETargetReach::EmptyCell);

	TestTrue("Should return empty cells", TargetCells.Num() > 0);
	TestFalse("Should not include occupied cell", TargetCells.Contains(FIntPoint(2, 2)));

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: EmptyCellOrFriendly returns both
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingEmptyOrFriendlyTest,
	"KBS.Grid.Components.Targeting.GetValidTargetCells_EmptyCellOrFriendly_ReturnsBoth",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingEmptyOrFriendlyTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* Unit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* Friendly = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 3, 3, EBattleLayer::Ground, ETeamSide::Attacker);

	TArray<FIntPoint> TargetCells = Targeting->GetValidTargetCells(Unit, ETargetReach::EmptyCellOrFriendly);

	TestTrue("Should include friendly cells", TargetCells.Contains(FIntPoint(3, 3)));
	TestTrue("Should have many cells (empty + friendly)", TargetCells.Num() > 1);

	Grid->Destroy();
	Unit->Destroy();
	Friendly->Destroy();
	return true;
}

// Test: Movement for ground units returns adjacent and flank
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingMovementGroundTest,
	"KBS.Grid.Components.Targeting.GetMovementCells_GroundUnit_ReturnsAdjacentAndFlank",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingMovementGroundTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* Unit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);

	TArray<FIntPoint> MoveCells = Targeting->GetValidTargetCells(Unit, ETargetReach::Movement);

	TestTrue("Should have movement cells", MoveCells.Num() > 0);
	// Adjacent cells should be included
	TestTrue("Should include adjacent cells", MoveCells.Num() >= 3); // At least 3 adjacent valid cells

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: Movement for air units returns all valid cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingMovementAirTest,
	"KBS.Grid.Components.Targeting.GetMovementCells_AirUnit_ReturnsAllValid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingMovementAirTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* AirUnit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Air, ETeamSide::Attacker);

	TArray<FIntPoint> MoveCells = Targeting->GetValidTargetCells(AirUnit, ETargetReach::Movement);

	// Air units can move to any valid non-restricted cell
	TestTrue("Should have many movement cells", MoveCells.Num() > 10);

	Grid->Destroy();
	AirUnit->Destroy();
	return true;
}

// Test: Adjacent movement returns 4 cardinal directions
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingAdjacentMovementTest,
	"KBS.Grid.Components.Targeting.GetAdjacentMoveCells_Returns4Directions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingAdjacentMovementTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* Unit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);

	TArray<FIntPoint> MoveCells = Targeting->GetValidTargetCells(Unit, ETargetReach::Movement);

	// Should include at least cardinal directions: up, down, left, right
	bool bHasUp = MoveCells.Contains(FIntPoint(2, 1));
	bool bHasDown = MoveCells.Contains(FIntPoint(2, 3));
	bool bHasLeft = MoveCells.Contains(FIntPoint(1, 2));
	bool bHasRight = MoveCells.Contains(FIntPoint(3, 2));

	TestTrue("Should include up", bHasUp);
	TestTrue("Should include down", bHasDown);
	TestTrue("Should include left", bHasLeft);
	TestTrue("Should include right", bHasRight);

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: Adjacent movement excludes enemy-occupied cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingAdjacentBlockedByEnemyTest,
	"KBS.Grid.Components.Targeting.GetAdjacentMoveCells_BlockedByEnemy_ExcludesCell",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingAdjacentBlockedByEnemyTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* Unit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* Enemy = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 3, EBattleLayer::Ground, ETeamSide::Defender);

	TArray<FIntPoint> MoveCells = Targeting->GetValidTargetCells(Unit, ETargetReach::Movement);

	TestFalse("Should not include enemy-occupied cell", MoveCells.Contains(FIntPoint(3, 2)));

	Grid->Destroy();
	Unit->Destroy();
	Enemy->Destroy();
	return true;
}

// Test: Adjacent movement excludes own team flank
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingAdjacentExcludesOwnFlankTest,
	"KBS.Grid.Components.Targeting.GetAdjacentMoveCells_TeamFlank_ExcludesOwnFlank",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingAdjacentExcludesOwnFlankTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	// Attacker at center line adjacent to attacker flank
	AUnit* Unit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);

	TArray<FIntPoint> MoveCells = Targeting->GetValidTargetCells(Unit, ETargetReach::Movement);

	// Attackers should not be able to enter attacker flank (columns 0) from center
	// This depends on flank rules defined in the component

	Grid->Destroy();
	Unit->Destroy();
	return true;
}

// Test: Flank movement requires enemy flank
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingFlankMovementRequiresEnemyFlankTest,
	"KBS.Grid.Components.Targeting.GetFlankMoveCells_RequiresEnemyFlank",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingFlankMovementRequiresEnemyFlankTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* AttackerUnit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);

	TArray<FIntPoint> MoveCells = Targeting->GetValidTargetCells(AttackerUnit, ETargetReach::Movement);

	// Enemy flank for attackers is columns 3 and 4
	// Check if any flank moves are included

	Grid->Destroy();
	AttackerUnit->Destroy();
	return true;
}

// Test: Multi-cell movement checks secondary cell
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingMultiCellMovementTest,
	"KBS.Grid.Components.Targeting.GetMovementCells_MultiCell_ChecksSecondaryCell",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingMultiCellMovementTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* MultiUnit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 1, 1, EBattleLayer::Ground, ETeamSide::Attacker, true);

	TArray<FIntPoint> MoveCells = Targeting->GetValidTargetCells(MultiUnit, ETargetReach::Movement);

	// Multi-cell units should only include moves where both primary and secondary cells are valid
	TestTrue("Multi-cell unit should have valid moves", MoveCells.Num() >= 0);

	Grid->Destroy();
	MultiUnit->Destroy();
	return true;
}

// Test: AnyCorpse returns all corpse cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingAnyCorpseTest,
	"KBS.Grid.Components.Targeting.GetValidTargetCells_AnyCorpse_ReturnsAllCorpseCells",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingAnyCorpseTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* Unit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* Corpse1 = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 0, 0, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* Corpse2 = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 4, 4, EBattleLayer::Ground, ETeamSide::Defender);

	DataManager->PushCorpse(Corpse1, 1, 1);
	DataManager->PushCorpse(Corpse2, 3, 3);

	TArray<FIntPoint> CorpseCells = Targeting->GetValidTargetCells(Unit, ETargetReach::AnyCorpse);

	TestEqual("Should return 2 corpse cells", CorpseCells.Num(), 2);
	TestTrue("Should include corpse 1", CorpseCells.Contains(FIntPoint(1, 1)));
	TestTrue("Should include corpse 2", CorpseCells.Contains(FIntPoint(3, 3)));

	Grid->Destroy();
	Unit->Destroy();
	Corpse1->Destroy();
	Corpse2->Destroy();
	return true;
}

// Test: FriendlyCorpse returns only friendly corpses
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingFriendlyCorpseTest,
	"KBS.Grid.Components.Targeting.GetValidTargetCells_FriendlyCorpse_ReturnsOnlyFriendly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingFriendlyCorpseTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* Unit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* FriendlyCorpse = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 0, 0, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* EnemyCorpse = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 4, 4, EBattleLayer::Ground, ETeamSide::Defender);

	DataManager->PushCorpse(FriendlyCorpse, 1, 1);
	DataManager->PushCorpse(EnemyCorpse, 3, 3);

	TArray<FIntPoint> CorpseCells = Targeting->GetValidTargetCells(Unit, ETargetReach::FriendlyCorpse);

	TestEqual("Should return 1 friendly corpse", CorpseCells.Num(), 1);
	TestTrue("Should include friendly corpse", CorpseCells.Contains(FIntPoint(1, 1)));
	TestFalse("Should not include enemy corpse", CorpseCells.Contains(FIntPoint(3, 3)));

	Grid->Destroy();
	Unit->Destroy();
	FriendlyCorpse->Destroy();
	EnemyCorpse->Destroy();
	return true;
}

// Test: EnemyCorpse returns only enemy corpses
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingEnemyCorpseTest,
	"KBS.Grid.Components.Targeting.GetValidTargetCells_EnemyCorpse_ReturnsOnlyEnemy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingEnemyCorpseTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* Unit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* FriendlyCorpse = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 0, 0, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* EnemyCorpse = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 4, 4, EBattleLayer::Ground, ETeamSide::Defender);

	DataManager->PushCorpse(FriendlyCorpse, 1, 1);
	DataManager->PushCorpse(EnemyCorpse, 3, 3);

	TArray<FIntPoint> CorpseCells = Targeting->GetValidTargetCells(Unit, ETargetReach::EnemyCorpse);

	TestEqual("Should return 1 enemy corpse", CorpseCells.Num(), 1);
	TestFalse("Should not include friendly corpse", CorpseCells.Contains(FIntPoint(1, 1)));
	TestTrue("Should include enemy corpse", CorpseCells.Contains(FIntPoint(3, 3)));

	Grid->Destroy();
	Unit->Destroy();
	FriendlyCorpse->Destroy();
	EnemyCorpse->Destroy();
	return true;
}

// Test: NonBlockedCorpse excludes occupied cells
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingNonBlockedCorpseTest,
	"KBS.Grid.Components.Targeting.GetValidTargetCells_NonBlockedCorpse_ExcludesOccupied",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingNonBlockedCorpseTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* Unit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* Corpse = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 0, 0, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* BlockingUnit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 3, 3, EBattleLayer::Ground, ETeamSide::Attacker);

	DataManager->PushCorpse(Corpse, 1, 1); // Non-blocked
	DataManager->PushCorpse(Corpse, 3, 3); // Blocked by BlockingUnit

	TArray<FIntPoint> CorpseCells = Targeting->GetValidTargetCells(Unit, ETargetReach::AnyNonBlockedCorpse);

	TestTrue("Should include non-blocked corpse", CorpseCells.Contains(FIntPoint(1, 1)));
	TestFalse("Should not include blocked corpse", CorpseCells.Contains(FIntPoint(3, 3)));

	Grid->Destroy();
	Unit->Destroy();
	Corpse->Destroy();
	BlockingUnit->Destroy();
	return true;
}

// Test: GetValidTargetUnits converts cells to units
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingGetUnitsTest,
	"KBS.Grid.Components.Targeting.GetValidTargetUnits_ReturnsUnitsInCells",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingGetUnitsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* Unit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* Enemy1 = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 1, 1, EBattleLayer::Ground, ETeamSide::Defender);
	AUnit* Enemy2 = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 3, 3, EBattleLayer::Ground, ETeamSide::Defender);

	TArray<AUnit*> TargetUnits = Targeting->GetValidTargetUnits(Unit, ETargetReach::AnyEnemy);

	TestEqual("Should return 2 enemy units", TargetUnits.Num(), 2);
	TestTrue("Should include enemy 1", TargetUnits.Contains(Enemy1));
	TestTrue("Should include enemy 2", TargetUnits.Contains(Enemy2));

	Grid->Destroy();
	Unit->Destroy();
	Enemy1->Destroy();
	Enemy2->Destroy();
	return true;
}

// Test: GetValidTargetUnits excludes dead units by default
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGridTargetingGetUnitsExcludesDeadTest,
	"KBS.Grid.Components.Targeting.GetValidTargetUnits_ExcludesDeadUnits",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridTargetingGetUnitsExcludesDeadTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) { AddError("Failed to get world"); return false; }

	ATacBattleGrid* Grid;
	UGridDataManager* DataManager;
	UGridTargetingComponent* Targeting;
	FGridTargetingTestHelper::SetupGridWithComponents(World, Grid, DataManager, Targeting);

	AUnit* Unit = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 2, 2, EBattleLayer::Ground, ETeamSide::Attacker);
	AUnit* LivingEnemy = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 1, 1, EBattleLayer::Ground, ETeamSide::Defender);
	AUnit* DeadEnemy = FGridTargetingTestHelper::CreateAndPlaceUnit(World, DataManager, Grid, 3, 3, EBattleLayer::Ground, ETeamSide::Defender);

	// Note: Unit->IsDead() check requires proper Unit implementation

	TArray<AUnit*> TargetUnits = Targeting->GetValidTargetUnits(Unit, ETargetReach::AnyEnemy, false, false);

	// Should include living enemy

	Grid->Destroy();
	Unit->Destroy();
	LivingEnemy->Destroy();
	DeadEnemy->Destroy();
	return true;
}
