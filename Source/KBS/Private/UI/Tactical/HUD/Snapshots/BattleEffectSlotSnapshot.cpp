#include "UI/Tactical/HUD/Snapshots/BattleEffectSlotSnapshot.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "GameplayTypes/EffectTypes.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UBattleEffectSlotSnapshot::SetupFromEffect(UBattleEffect* Effect)
{
	if (!Effect)
	{
		Clear();
		return;
	}

	UBattleEffectDataAsset* Config = Effect->GetConfig();
	if (!Config)
	{
		Clear();
		return;
	}

	// Set icon
	if (EffectIcon && !Config->Image.IsNull())
	{
		UTexture2D* IconTexture = Config->Image.LoadSynchronous();
		if (IconTexture)
		{
			EffectIcon->SetBrushFromTexture(IconTexture);
			EffectIcon->SetVisibility(ESlateVisibility::Visible);
		}
	}

	// Set duration
	if (DurationText)
	{
		const int32 Duration = Effect->GetDuration();
		if (Duration > 0)
		{
			DurationText->SetText(FText::AsNumber(Duration));
		}
		else
		{
			DurationText->SetText(FText::FromString(TEXT("∞")));
		}
		DurationText->SetVisibility(ESlateVisibility::Visible);
	}

	// Set frame color based on effect type
	if (EffectFrameBorder)
	{
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

	// Set tooltip
	FText TooltipText = FText::Format(
		FText::FromString(TEXT("{0}\n{1}")),
		Config->Name,
		Config->Description
	);
	SetToolTipText(TooltipText);

	SetVisibility(ESlateVisibility::Visible);
}

void UBattleEffectSlotSnapshot::Clear()
{
	if (EffectIcon)
	{
		EffectIcon->SetBrushFromTexture(nullptr);
		EffectIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (DurationText)
	{
		DurationText->SetText(FText::GetEmpty());
		DurationText->SetVisibility(ESlateVisibility::Collapsed);
	}

	SetToolTipText(FText::GetEmpty());
	SetVisibility(ESlateVisibility::Collapsed);
}
