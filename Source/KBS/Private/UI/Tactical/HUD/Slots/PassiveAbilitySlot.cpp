#include "UI/Tactical/HUD/Slots/PassiveAbilitySlot.h"
#include "Components/Image.h"
#include "Blueprint/WidgetTree.h"
#include "GameMechanics/Units/Abilities/AbilityDisplayData.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"

void UPassiveAbilitySlot::NativeConstruct()
{
	Super::NativeConstruct();

	// Create AbilityIcon image
	AbilityIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	WidgetTree->RootWidget = AbilityIcon;
}

void UPassiveAbilitySlot::NativeDestruct()
{
	Clear();
	Super::NativeDestruct();
}

void UPassiveAbilitySlot::SetupFromAbility(UUnitAbilityInstance* Ability, bool bBindToEvents)
{
	// Clear previous state
	Clear();

	if (!Ability)
	{
		return;
	}

	BoundAbility = Ability;
	bIsListeningToEvents = bBindToEvents;

	// Bind to events if requested
	if (bIsListeningToEvents && BoundAbility)
	{
		BoundAbility->OnAbilityUsed.AddDynamic(this, &UPassiveAbilitySlot::OnAbilityTriggered);
	}

	// Get and apply display data
	FAbilityDisplayData DisplayData = BoundAbility->GetAbilityDisplayData();

	// Set icon texture
	if (AbilityIcon && DisplayData.Icon)
	{
		AbilityIcon->SetBrushFromTexture(DisplayData.Icon);
	}
	SetVisibility(ESlateVisibility::Visible);
}

void UPassiveAbilitySlot::Clear()
{
	// Unbind from current ability
	if (BoundAbility && bIsListeningToEvents)
	{
		BoundAbility->OnAbilityUsed.RemoveDynamic(this, &UPassiveAbilitySlot::OnAbilityTriggered);
	}

	BoundAbility = nullptr;
	bIsListeningToEvents = false;

	SetVisibility(ESlateVisibility::Collapsed);
}

void UPassiveAbilitySlot::OnAbilityTriggered(int32 ChargesLeft, bool bAvailable)
{
	// Only process if we're listening to events
	if (!bIsListeningToEvents || !BoundAbility)
	{
		return;
	}

	// Call Blueprint hook for flash VFX
	BP_OnAbilityTriggered(BoundAbility);
}
