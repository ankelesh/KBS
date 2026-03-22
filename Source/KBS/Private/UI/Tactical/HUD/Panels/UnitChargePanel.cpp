#include "UI/Tactical/HUD/Panels/UnitChargePanel.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Components/UnitChargeComponent.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "UI/Tactical/HUD/Slots/UnitChargeSlot.h"
#include "Components/HorizontalBox.h"

void UUnitChargePanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWorld* World = GetWorld())
	{
		TurnSubsystem = World->GetSubsystem<UTacTurnSubsystem>();
		if (TurnSubsystem)
		{
			TurnSubsystem->OnTurnStart.AddDynamic(this, &UUnitChargePanel::OnTurnStarted);
			if (AUnit* Unit = TurnSubsystem->GetCurrentUnit())
			{
				SetupForUnit(Unit);
			}
		}
	}
}

void UUnitChargePanel::NativeDestruct()
{
	UnbindFromCurrentComponent();

	if (TurnSubsystem)
	{
		TurnSubsystem->OnTurnStart.RemoveDynamic(this, &UUnitChargePanel::OnTurnStarted);
	}

	Super::NativeDestruct();
}

void UUnitChargePanel::Clear()
{
	UnbindFromCurrentComponent();

	if (ChargeSlotContainer)
	{
		ChargeSlotContainer->ClearChildren();
	}

	SlotMap.Empty();
	CurrentUnit = nullptr;
}

void UUnitChargePanel::OnTurnStarted(AUnit* Unit)
{
	SetupForUnit(Unit);
}

void UUnitChargePanel::OnChargeChangedHandler(const FGameplayTag& Tag, int32 NewValue)
{
	if (UUnitChargeSlot** Slot = SlotMap.Find(Tag))
	{
		(*Slot)->UpdateValue(NewValue);
	}

	BP_OnChargeChanged(Tag, NewValue);
}

void UUnitChargePanel::SetupForUnit(AUnit* Unit)
{
	Clear();

	if (!Unit)
	{
		BP_OnHidden();
		return;
	}

	UUnitChargeComponent* ChargeComponent = Unit->FindComponentByClass<UUnitChargeComponent>();
	if (!ChargeComponent)
	{
		BP_OnHidden();
		return;
	}

	CurrentUnit = Unit;
	BoundComponent = ChargeComponent;
	BoundComponent->OnChargeChanged.AddDynamic(this, &UUnitChargePanel::OnChargeChangedHandler);

	RebuildSlots(ChargeComponent);
	BP_OnUnitSet(Unit);
}

void UUnitChargePanel::RebuildSlots(UUnitChargeComponent* Component)
{
	if (!ChargeSlotContainer || !ChargeSlotClass)
	{
		return;
	}

	ChargeSlotContainer->ClearChildren();
	SlotMap.Empty();

	TArray<FGameplayTag> Tags = TagsToDisplay.Num() > 0
		? TagsToDisplay
		: Component->GetTrackedTags();

	if (TagsToDisplay.Num() == 0)
	{
		Tags.Sort([](const FGameplayTag& A, const FGameplayTag& B)
		{
			return A.ToString() < B.ToString();
		});
	}

	for (const FGameplayTag& Tag : Tags)
	{
		UUnitChargeSlot* Slot = CreateWidget<UUnitChargeSlot>(this, ChargeSlotClass);
		if (!Slot)
		{
			continue;
		}

		Slot->SetupSlot(Tag, Component->GetCharge(Tag), Component->GetChargeName(Tag), nullptr);
		ChargeSlotContainer->AddChild(Slot);
		SlotMap.Add(Tag, Slot);
	}

	BP_OnCountersRefreshed(SlotMap.Num());
}

void UUnitChargePanel::UnbindFromCurrentComponent()
{
	if (BoundComponent)
	{
		BoundComponent->OnChargeChanged.RemoveDynamic(this, &UUnitChargePanel::OnChargeChangedHandler);
		BoundComponent = nullptr;
	}
}
