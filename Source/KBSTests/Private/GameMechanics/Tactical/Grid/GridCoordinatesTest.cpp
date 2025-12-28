#include "Misc/AutomationTest.h"
#include "GameplayTypes/GridCoordinates.h"

// Test: Valid cell detection
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGridCoordinatesValidCellTest,
    "KBS.Grid.Coordinates.ValidCell",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridCoordinatesValidCellTest::RunTest(const FString& Parameters)
{
    // Valid cells
    TestTrue("Center (2,2) is valid", FGridCoordinates::IsValidCell(2, 2));
    TestTrue("Corner (0,0) is valid", FGridCoordinates::IsValidCell(0, 0));
    TestTrue("Corner (4,4) is valid", FGridCoordinates::IsValidCell(4, 4));
    
    // Out of bounds
    TestFalse("Negative row invalid", FGridCoordinates::IsValidCell(-1, 2));
    TestFalse("Row >= 5 invalid", FGridCoordinates::IsValidCell(5, 2));
    TestFalse("Col >= 5 invalid", FGridCoordinates::IsValidCell(2, 5));
    
    // Restricted cells (center row flanks)
    TestFalse("(2,0) is restricted", FGridCoordinates::IsValidCell(2, 0));
    TestFalse("(2,4) is restricted", FGridCoordinates::IsValidCell(2, 4));
    
    return true;
}

// Test: Flank cell detection
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGridCoordinatesFlankCellTest,
    "KBS.Grid.Coordinates.FlankCell",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FGridCoordinatesFlankCellTest::RunTest(const FString& Parameters)
{
    // Flank cells (col 0 or 4, NOT center row)
    TestTrue("(0,0) is flank", FGridCoordinates::IsFlankCell(0, 0));
    TestTrue("(1,4) is flank", FGridCoordinates::IsFlankCell(1, 4));
    TestTrue("(4,0) is flank", FGridCoordinates::IsFlankCell(4, 0));
    
    // Center row flanks are restricted, NOT flank
    TestFalse("(2,0) not flank", FGridCoordinates::IsFlankCell(2, 0));
    TestFalse("(2,4) not flank", FGridCoordinates::IsFlankCell(2, 4));
    
    // Normal cells
    TestFalse("(2,2) not flank", FGridCoordinates::IsFlankCell(2, 2));
    
    return true;
}