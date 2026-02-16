#include "UI/Tactical/HUD/Slots/UnitTeamSlot.h"
#include "UI/Tactical/HUD/TacticalHUD.h"
#include "UI/Tactical/Controller/TacticalGameController.h"
#include "GameMechanics/Units/Unit.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UUnitTeamSlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUnitTeamSlot::NativeDestruct()
{
	Clear();
	Super::NativeDestruct();
}

void UUnitTeamSlot::SetupUnit(AUnit* InUnit)
{
	check(InUnit);

	// Unbind from previous unit if any
	if (AUnit* PreviousUnit = BoundUnit.Get())
	{
		PreviousUnit->OnUnitDamaged.RemoveDynamic(this, &UUnitTeamSlot::OnUnitDamaged);
		PreviousUnit->OnUnitDied.RemoveDynamic(this, &UUnitTeamSlot::OnUnitDied);
	}

	BoundUnit = InUnit;

	// Bind to unit events
	InUnit->OnUnitDamaged.AddDynamic(this, &UUnitTeamSlot::OnUnitDamaged);
	InUnit->OnUnitDied.AddDynamic(this, &UUnitTeamSlot::OnUnitDied);

	// Refresh display with current unit data
	RefreshDisplay();
	SetVisibility(ESlateVisibility::Visible);
}

void UUnitTeamSlot::Clear()
{
	if (AUnit* Unit = BoundUnit.Get())
	{
		Unit->OnUnitDamaged.RemoveDynamic(this, &UUnitTeamSlot::OnUnitDamaged);
		Unit->OnUnitDied.RemoveDynamic(this, &UUnitTeamSlot::OnUnitDied);
	}
	BoundUnit = nullptr;

	SetVisibility(ESlateVisibility::Collapsed);
}

FReply UUnitTeamSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		if (AUnit* Unit = BoundUnit.Get())
		{
			UTacticalHUD* TacticalHUD = GetTacticalHUD();
			check(TacticalHUD);
			TacticalHUD->ShowUnitDetailsPopup(Unit);
			return FReply::Handled();
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UUnitTeamSlot::OnUnitDamaged(AUnit* Victim, AUnit* Attacker)
{
	UpdateHPOverlay();
	BP_OnUnitDamaged();
}

void UUnitTeamSlot::OnUnitDied(AUnit* Unit)
{
	ApplyDeathVisuals();
	BP_OnUnitDied();
}

void UUnitTeamSlot::RefreshDisplay()
{
	AUnit* Unit = BoundUnit.Get();
	if (!Unit) return;

	FUnitDisplayData DisplayData = Unit->GetDisplayData();

	// Update name
	Name->SetText(FText::FromString(DisplayData.UnitName));

	// Update portrait
	if (DisplayData.PortraitTexture)
	{
		Portrait->SetBrushFromTexture(DisplayData.PortraitTexture);
	}

	// Update frame color based on team
	UpdateFrameColor();

	// Update HP overlay
	UpdateHPOverlay();
}

void UUnitTeamSlot::UpdateHPOverlay()
{
	AUnit* Unit = BoundUnit.Get();
	if (!Unit) return;

	FUnitDisplayData DisplayData = Unit->GetDisplayData();
	float HealthPercent = DisplayData.MaxHealth > 0.0f
		? DisplayData.CurrentHealth / DisplayData.MaxHealth
		: 0.0f;

	// HP overlay grows from bottom up as unit takes damage
	// 100% health = 0% overlay height, 0% health = 100% overlay height
	float DamagePercent = 1.0f - HealthPercent;

	SetHPOverlayHeight(DamagePercent);

	// Set overlay color (will be overridden by death color if unit is dead)
	HPOverlay->SetColorAndOpacity(OverlayColor);
}

void UUnitTeamSlot::UpdateFrameColor()
{
	AUnit* Unit = BoundUnit.Get();
	if (!Unit) return;

	// Defender team gets different frame color
	FLinearColor FrameColor = (Unit->GetTeamSide() == ETeamSide::Defender)
		? DefenderFrameColor
		: AttackerFrameColor;

	Frame->SetColorAndOpacity(FrameColor);
}

void UUnitTeamSlot::SetHPOverlayHeight(float HealthPercent)
{
	// Clamp to [0, 1]
	HealthPercent = FMath::Clamp(HealthPercent, 0.0f, 1.0f);

	// Scale overlay from bottom up by adjusting render transform
	// Anchor the overlay to bottom and scale Y
	FWidgetTransform Transform;
	Transform.Scale = FVector2D(1.0f, HealthPercent);
	HPOverlay->SetRenderTransform(Transform);

	// Set render transform pivot to bottom (0.5, 1.0) so it grows upward
	HPOverlay->SetRenderTransformPivot(FVector2D(0.5f, 1.0f));
}

void UUnitTeamSlot::ApplyDeathVisuals()
{
	// Fill entire portrait with death color
	SetHPOverlayHeight(1.0f);
	HPOverlay->SetColorAndOpacity(OverlayDeathColor);
}

UTacticalHUD* UUnitTeamSlot::GetTacticalHUD() const
{
	APlayerController* PC = GetOwningPlayer();
	check(PC);

	ATacticalGameController* TacticalController = Cast<ATacticalGameController>(PC);
	check(TacticalController);

	return TacticalController->GetTacticalHUD();
}
