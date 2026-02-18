#include "UI/Tactical/HUD/Panels/TurnQueuePanel.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "GameMechanics/Units/Unit.h"
#include "UI/Tactical/HUD/Slots/UnitTurnQueueSlot.h"
#include "UI/Tactical/HUD/TacticalHUD.h"
#include "Components/VerticalBox.h"
#include "UI/Tactical/Controller/TacticalGameController.h"

void UTurnQueuePanel::NativeConstruct()
{
	Super::NativeConstruct();

	// Get turn subsystem
	if (UWorld* World = GetWorld())
	{
		TurnSubsystem = World->GetSubsystem<UTacTurnSubsystem>();
		checkf(TurnSubsystem, TEXT("UTurnQueuePanel::NativeConstruct - TurnSubsystem not found"));
	}

	// Get owning HUD
	ATacticalGameController* TacticalController = Cast<ATacticalGameController>(GetOwningPlayer());
	checkf(TacticalController, TEXT("USpellbookSlot::OnSpellbookButtonClicked - TacticalGameController is null"));

	OwningHUD = TacticalController->GetTacticalHUD();
	checkf(OwningHUD, TEXT("USpellbookSlot::OnSpellbookButtonClicked - TacticalHUD is null"));

	// Pre-allocate current unit slot
	CurrentUnitSlot = CreateWidget<UUnitTurnQueueSlot>(this, UnitTurnQueueSlotClass);
	CurrentUnitContainer->AddChild(CurrentUnitSlot);
	CurrentUnitSlot->OnDetailsRequested.AddDynamic(this, &UTurnQueuePanel::OnUnitDetailsRequested);
	CurrentUnitSlot->SetVisibility(ESlateVisibility::Collapsed);

	// Pre-allocate all 10 queue slots
	QueueSlots.Reserve(MAX_QUEUE_SLOTS);
	for (int32 i = 0; i < MAX_QUEUE_SLOTS; ++i)
	{
		UUnitTurnQueueSlot* UtqSlot = CreateWidget<UUnitTurnQueueSlot>(this, UnitTurnQueueSlotClass);
		QueueContainer->AddChild(UtqSlot);
		UtqSlot->OnDetailsRequested.AddDynamic(this, &UTurnQueuePanel::OnUnitDetailsRequested);
		UtqSlot->SetVisibility(ESlateVisibility::Collapsed);
		QueueSlots.Add(UtqSlot);
	}

	// Bind to turn start event and sync initial state after slots are ready
	TurnSubsystem->OnTurnStart.AddDynamic(this, &UTurnQueuePanel::OnTurnStart);
	if (TurnSubsystem->GetCurrentUnit())
	{
		UpdatePanel();
	}
}

void UTurnQueuePanel::NativeDestruct()
{
	TurnSubsystem->OnTurnStart.RemoveDynamic(this, &UTurnQueuePanel::OnTurnStart);

	CurrentUnitSlot->OnDetailsRequested.RemoveDynamic(this, &UTurnQueuePanel::OnUnitDetailsRequested);

	for (UUnitTurnQueueSlot* UtqSlot : QueueSlots)
	{
		UtqSlot->OnDetailsRequested.RemoveDynamic(this, &UTurnQueuePanel::OnUnitDetailsRequested);
	}

	Clear();

	Super::NativeDestruct();
}

void UTurnQueuePanel::OnTurnStart(AUnit* Unit)
{
	UpdatePanel();
}

void UTurnQueuePanel::OnUnitDetailsRequested(AUnit* Unit)
{
	if (OwningHUD)
	{
		OwningHUD->ShowUnitDetailsPopup(Unit);
	}
}

void UTurnQueuePanel::UpdatePanel()
{
	AUnit* CurrentUnit = TurnSubsystem->GetCurrentUnit();
	UpdateCurrentUnitSlot(CurrentUnit);

	TArray<AUnit*> RemainingUnits = TurnSubsystem->GetRemainingUnits(MAX_QUEUE_SLOTS);
	UpdateQueueSlots(RemainingUnits);
}

void UTurnQueuePanel::UpdateCurrentUnitSlot(AUnit* CurrentUnit)
{
	if (CurrentUnit)
	{
		int32 Initiative = TurnSubsystem->GetUnitInitiative(CurrentUnit);
		CurrentUnitSlot->SetupSlot(CurrentUnit, Initiative, true);
		CurrentUnitSlot->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		CurrentUnitSlot->Clear();
		CurrentUnitSlot->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UTurnQueuePanel::UpdateQueueSlots(const TArray<AUnit*>& RemainingUnits)
{
	for (int32 i = 0; i < MAX_QUEUE_SLOTS; ++i)
	{
		if (i < RemainingUnits.Num() && RemainingUnits[i])
		{
			AUnit* Unit = RemainingUnits[i];
			int32 Initiative = TurnSubsystem->GetUnitInitiative(Unit);
			QueueSlots[i]->SetupSlot(Unit, Initiative, false);
			QueueSlots[i]->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			QueueSlots[i]->Clear();
			QueueSlots[i]->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UTurnQueuePanel::Clear()
{
	CurrentUnitSlot->Clear();

	for (UUnitTurnQueueSlot* UtqSlot : QueueSlots)
	{
		UtqSlot->Clear();
	}
}
