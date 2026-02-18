#include "UI/Tactical/HUD/Tables/TeamTable.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Units/Unit.h"
#include "UI/Tactical/HUD/Slots/UnitTeamSlot.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

void UTeamTable::NativeConstruct()
{
	Super::NativeConstruct();

	checkf(SlotClass, TEXT("SlotClass not set in TeamTable"));
}

void UTeamTable::NativeDestruct()
{
	Clear();
	Super::NativeDestruct();
}

void UTeamTable::SetTeam(UBattleTeam* InTeam)
{
	Clear();

	BoundTeam = InTeam;

	const TArray<TObjectPtr<AUnit>>& Units = BoundTeam->GetUnits();
	for (int32 i = 0; i < Units.Num(); ++i)
	{
		UUnitTeamSlot* UtSlot = CreateWidget<UUnitTeamSlot>(this, SlotClass);
		UtSlot->SetupUnit(Units[i]);
		EmplaceSlot(UtSlot, i);
		Slots.Add(UtSlot);
	}
}

void UTeamTable::Clear()
{
	UnitGrid->ClearChildren();
	Slots.Empty();
	BoundTeam = nullptr;
}

void UTeamTable::EmplaceSlot(UUnitTeamSlot* UtSlot, int32 Index)
{
	UUniformGridSlot* GridSlot = UnitGrid->AddChildToUniformGrid(UtSlot);
	GridSlot->SetRow(Index / ColumnsPerRow);
	GridSlot->SetColumn(Index % ColumnsPerRow);
}
