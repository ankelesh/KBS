#include "UI/Picker/Slots/UnitRosterSlot.h"
#include "UI/Picker/DragDrop/UnitDragDropOperation.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "Components/TextBlock.h"

void UUnitRosterSlot::SetDefinition(UUnitDefinition* InDefinition)
{
	Definition = InDefinition;
	if (UnitNameLabel && Definition)
	{
		UnitNameLabel->SetText(FText::FromString(Definition->UnitName));
	}
}

FReply UUnitRosterSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		OnRightClicked.ExecuteIfBound(Definition);
		return FReply::Handled();
	}
	return UUserWidget::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UUnitRosterSlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	if (!Definition) return;
	UUnitDragDropOperation* Op = NewObject<UUnitDragDropOperation>(this);
	Op->Definition = Definition;
	Op->DefaultDragVisual = this;
	OutOperation = Op;
}
