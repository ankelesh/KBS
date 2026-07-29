#include "UI/Picker/Panels/TeamGridPanel.h"
#include "UI/Picker/Slots/TeamUnitSlot.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"

void UTeamGridPanel::NativeConstruct()
{
	Super::NativeConstruct();
	checkf(SlotClass, TEXT("UTeamGridPanel: SlotClass not set in Blueprint defaults"));

	CreatedSlots.SetNum(Rows * Cols);
	for (int32 Row = 0; Row < Rows; ++Row)
	{
		for (int32 Col = 0; Col < Cols; ++Col)
		{
			UTeamUnitSlot* Slot = CreateWidget<UTeamUnitSlot>(GetOwningPlayer(), SlotClass);
			Slot->Setup(Row, Col, this);

			UGridSlot* GridSlot = SlotGrid->AddChildToGrid(Slot);
			GridSlot->SetRow(Row);
			GridSlot->SetColumn(Col);

			CreatedSlots[Row * Cols + Col] = Slot;
		}
	}
}

UTeamUnitSlot* UTeamGridPanel::GetSlot(int32 Row, int32 Col) const
{
	const int32 Index = Row * Cols + Col;
	return CreatedSlots.IsValidIndex(Index) ? CreatedSlots[Index].Get() : nullptr;
}

void UTeamGridPanel::RefreshFromSlots(const TArray<TObjectPtr<UUnitDefinition>>& Slots)
{
	for (int32 Row = 0; Row < Rows; ++Row)
	{
		UUnitDefinition* S0 = Slots[Row * Cols];
		UUnitDefinition* S1 = Slots[Row * Cols + 1];
		const bool b2Cell = S0 && S0 == S1;

		UTeamUnitSlot* Slot0 = GetSlot(Row, 0);
		UTeamUnitSlot* Slot1 = GetSlot(Row, 1);

		if (!Slot0 || !Slot1) continue;

		if (b2Cell)
		{
			Slot0->SetOccupied(S0);
			Slot0->SetSecondaryCell(false);
			Slot1->SetOccupied(S0);
			Slot1->SetSecondaryCell(true);
		}
		else
		{
			Slot0->SetSecondaryCell(false);
			Slot1->SetSecondaryCell(false);
			S0 ? Slot0->SetOccupied(S0) : Slot0->SetEmpty();
			S1 ? Slot1->SetOccupied(S1) : Slot1->SetEmpty();
		}
	}
}

void UTeamGridPanel::NotifyDrop(UUnitDefinition* Definition, int32 Row, int32 Col)
{
	OnUnitDropped.ExecuteIfBound(Definition, Row, Col);
}

void UTeamGridPanel::NotifyClear(int32 Row, int32 Col)
{
	OnSlotCleared.ExecuteIfBound(Row, Col);
}
