#include "UI/Tactical/Tables/TeamTable.h"
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
		UUnitTeamSlot* Slot = CreateWidget<UUnitTeamSlot>(this, SlotClass);
		Slot->SetupUnit(Units[i]);
		EmplaceSlot(Slot, i);
		Slots.Add(Slot);
	}
}

void UTeamTable::Clear()
{
	UnitGrid->ClearChildren();
	Slots.Empty();
	BoundTeam = nullptr;
}

void UTeamTable::EmplaceSlot(UUnitTeamSlot* Slot, int32 Index)
{
	UUniformGridSlot* GridSlot = UnitGrid->AddChildToUniformGrid(Slot);
	GridSlot->SetRow(Index / ColumnsPerRow);
	GridSlot->SetColumn(Index % ColumnsPerRow);
}
