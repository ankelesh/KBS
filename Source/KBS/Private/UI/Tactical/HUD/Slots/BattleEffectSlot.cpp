#include "UI/Tactical/HUD/Slots/BattleEffectSlot.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "GameplayTypes/EffectTypes.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "Components/Overlay.h"

void UBattleEffectSlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UBattleEffectSlot::NativeDestruct()
{
	Clear();
	Super::NativeDestruct();
}

void UBattleEffectSlot::SetupEffect(UBattleEffect* Effect)
{
	if (!Effect)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBattleEffectSlot::SetupEffect - Effect is null"));
		Clear();
		return;
	}

	// Unbind from previous effect if any
	if (BoundEffect)
	{
		BoundEffect->OnDurationChange.RemoveDynamic(this, &UBattleEffectSlot::OnEffectDurationChanged);
		BoundEffect->OnEffectRemoved.RemoveDynamic(this, &UBattleEffectSlot::OnEffectRemovedHandler);
	}

	BoundEffect = Effect;

	// Bind to effect events
	BoundEffect->OnDurationChange.AddDynamic(this, &UBattleEffectSlot::OnEffectDurationChanged);
	BoundEffect->OnEffectRemoved.AddDynamic(this, &UBattleEffectSlot::OnEffectRemovedHandler);

	// Update all visuals
	UpdateEffectIcon();
	UpdateEffectFrame();
	UpdateDurationDisplay();
	UpdateTooltip();

	// Make widget visible
	if (SlotRoot)
	{
		SlotRoot->SetVisibility(ESlateVisibility::Visible);
	}
}

void UBattleEffectSlot::Clear()
{
	// Unbind from effect
	if (BoundEffect)
	{
		BoundEffect->OnDurationChange.RemoveDynamic(this, &UBattleEffectSlot::OnEffectDurationChanged);
		BoundEffect->OnEffectRemoved.RemoveDynamic(this, &UBattleEffectSlot::OnEffectRemovedHandler);
		BoundEffect = nullptr;
	}

	// Clear visuals
	if (EffectIcon)
	{
		EffectIcon->SetBrushFromTexture(nullptr);
	}

	if (DurationText)
	{
		DurationText->SetText(FText::GetEmpty());
	}

	if (SlotRoot)
	{
		SlotRoot->SetVisibility(ESlateVisibility::Collapsed);
	}

	SetToolTipText(FText::GetEmpty());
}

void UBattleEffectSlot::OnEffectDurationChanged(int32 NewDuration)
{
	UpdateDurationDisplay();
}

void UBattleEffectSlot::OnEffectRemovedHandler(UBattleEffect* Effect)
{
	// Auto-clear when effect is removed
	Clear();
}

void UBattleEffectSlot::UpdateDurationDisplay()
{
	if (!BoundEffect || !DurationText)
	{
		return;
	}

	const int32 Duration = BoundEffect->GetDuration();

	// Display duration as number of turns remaining
	if (Duration > 0)
	{
		DurationText->SetText(FText::AsNumber(Duration));
	}
	else
	{
		// Show infinity symbol or empty for immutable/permanent effects
		DurationText->SetText(FText::FromString(TEXT("")));
	}
}

void UBattleEffectSlot::UpdateEffectFrame()
{
	if (!BoundEffect || !EffectFrameBorder)
	{
		return;
	}

	UBattleEffectDataAsset* Config = BoundEffect->GetConfig();
	if (!Config)
	{
		return;
	}

	FLinearColor FrameColor = NeutralFrameColor;

	switch (Config->EffectType)
	{
		case EEffectType::Positive:
			FrameColor = PositiveFrameColor;
			break;
		case EEffectType::Negative:
			FrameColor = NegativeFrameColor;
			break;
		case EEffectType::Neutral:
			FrameColor = NeutralFrameColor;
			break;
		case EEffectType::Immutable:
			FrameColor = ImmutableFrameColor;
			break;
	}

	EffectFrameBorder->SetBrushColor(FrameColor);
}

void UBattleEffectSlot::UpdateEffectIcon()
{
	if (!BoundEffect || !EffectIcon)
	{
		return;
	}

	UBattleEffectDataAsset* Config = BoundEffect->GetConfig();
	if (!Config)
	{
		return;
	}

	// Load icon texture if available
	if (!Config->Image.IsNull())
	{
		UTexture2D* IconTexture = Config->Image.LoadSynchronous();
		if (IconTexture)
		{
			EffectIcon->SetBrushFromTexture(IconTexture);
		}
	}
}

void UBattleEffectSlot::UpdateTooltip()
{
	if (!BoundEffect)
	{
		SetToolTipText(FText::GetEmpty());
		return;
	}

	UBattleEffectDataAsset* Config = BoundEffect->GetConfig();
	if (!Config)
	{
		SetToolTipText(FText::GetEmpty());
		return;
	}

	// Create tooltip with effect name and description
	FText TooltipText = FText::Format(
		FText::FromString(TEXT("{0}\n{1}")),
		Config->Name,
		Config->Description
	);

	SetToolTipText(TooltipText);
}
