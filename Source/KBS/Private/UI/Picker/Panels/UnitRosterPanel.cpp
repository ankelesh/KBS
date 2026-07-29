#include "UI/Picker/Panels/UnitRosterPanel.h"
#include "UI/Picker/Slots/UnitRosterSlot.h"
#include "Components/ScrollBox.h"

void UUnitRosterPanel::NativeConstruct()
{
	Super::NativeConstruct();
	checkf(SlotClass, TEXT("UUnitRosterPanel: SlotClass not set in Blueprint defaults"));

	for (UUnitDefinition* Def : UnitRoster)
	{
		if (!Def) continue;
		UUnitRosterSlot* Slot = CreateWidget<UUnitRosterSlot>(GetOwningPlayer(), SlotClass);
		Slot->SetDefinition(Def);
		Slot->OnRightClicked.BindLambda([this](UUnitDefinition* D) { OnUnitInfoRequested.ExecuteIfBound(D); });
		RosterScrollBox->AddChild(Slot);
	}
}
