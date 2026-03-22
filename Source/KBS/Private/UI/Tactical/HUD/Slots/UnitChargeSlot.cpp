#include "UI/Tactical/HUD/Slots/UnitChargeSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UUnitChargeSlot::SetupSlot(const FGameplayTag& Tag, int32 Value, const FText& Name, TSoftObjectPtr<UTexture2D> Icon)
{
	BoundTag = Tag;

	if (ChargeNameText)
	{
		ChargeNameText->SetText(Name);
	}

	if (ChargeValueText)
	{
		ChargeValueText->SetText(FText::AsNumber(Value));
	}

	if (ChargeIcon && !Icon.IsNull())
	{
		if (UTexture2D* LoadedIcon = Icon.LoadSynchronous())
		{
			ChargeIcon->SetBrushFromTexture(LoadedIcon);
		}
	}

	BP_OnSetup(Tag, Value, Name);
}

void UUnitChargeSlot::UpdateValue(int32 NewValue)
{
	if (ChargeValueText)
	{
		ChargeValueText->SetText(FText::AsNumber(NewValue));
	}

	BP_OnValueChanged(NewValue);
}
