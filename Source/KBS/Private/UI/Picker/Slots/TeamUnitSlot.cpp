#include "UI/Picker/Slots/TeamUnitSlot.h"
#include "UI/Picker/Panels/TeamGridPanel.h"
#include "UI/Picker/DragDrop/UnitDragDropOperation.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UTeamUnitSlot::Setup(int32 InRow, int32 InCol, UTeamGridPanel* InParent)
{
	SlotRow = InRow;
	SlotCol = InCol;
	ParentPanel = InParent;
}

void UTeamUnitSlot::SetOccupied(UUnitDefinition* Definition)
{
	if (UnitNameLabel)
	{
		UnitNameLabel->SetText(FText::FromString(Definition->UnitName));
		UnitNameLabel->SetVisibility(ESlateVisibility::Visible);
	}
	BP_OnSetOccupied(Definition->UnitName);
}

void UTeamUnitSlot::SetEmpty()
{
	if (UnitNameLabel)
	{
		UnitNameLabel->SetText(FText::GetEmpty());
	}
	BP_OnSetEmpty();
}

void UTeamUnitSlot::SetSecondaryCell(bool bIsSecondary)
{
	BP_OnSetSecondary(bIsSecondary);
}

bool UTeamUnitSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UUnitDragDropOperation* UnitOp = Cast<UUnitDragDropOperation>(InOperation);
	if (!UnitOp || !ParentPanel.IsValid()) return false;
	ParentPanel->NotifyDrop(UnitOp->Definition, SlotRow, SlotCol);
	BP_OnDragLeave();
	return true;
}

bool UTeamUnitSlot::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	BP_OnDragEnter(Cast<UUnitDragDropOperation>(InOperation) != nullptr);
	return true;
}

void UTeamUnitSlot::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	BP_OnDragLeave();
}

FReply UTeamUnitSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton) && ParentPanel.IsValid())
	{
		ParentPanel->NotifyClear(SlotRow, SlotCol);
		return FReply::Handled();
	}
	return UUserWidget::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
