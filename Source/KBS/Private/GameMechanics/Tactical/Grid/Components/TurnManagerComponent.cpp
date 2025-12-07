#include "GameMechanics/Tactical/Grid/Components/TurnManagerComponent.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Units/UnitStats.h"

UTurnManagerComponent::UTurnManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTurnManagerComponent::StartBattle(const TArray<AUnit*>& Units)
{
	AllUnits.Empty();
	for (AUnit* Unit : Units)
	{
		if (Unit)
		{
			AllUnits.Add(Unit);
		}
	}

	bBattleActive = true;
	CurrentTurnNumber = 1;
	ActiveUnit = nullptr;

	BuildInitiativeQueue();
	StartGlobalTurn();
}

void UTurnManagerComponent::EndBattle()
{
	bBattleActive = false;
	TurnQueue.Empty();
	ActiveUnit = nullptr;

	UE_LOG(LogTemp, Log, TEXT("Battle ended at turn %d"), CurrentTurnNumber);
}

void UTurnManagerComponent::BuildInitiativeQueue()
{
	TurnQueue.Empty();

	for (AUnit* Unit : AllUnits)
	{
		if (!Unit || Unit->GetCurrentHealth() <= 0.0f)
		{
			continue;
		}

		FTurnQueueEntry Entry;
		Entry.Unit = Unit;
		Entry.InitiativeRoll = RollInitiative();
		Entry.TotalInitiative = Unit->GetModifiedStats().Initiative + Entry.InitiativeRoll;

		TurnQueue.Add(Entry);

		UE_LOG(LogTemp, Log, TEXT("Unit %s - Initiative: %d (Base: %d + Roll: %d)"),
			*Unit->GetName(),
			Entry.TotalInitiative,
			Unit->GetModifiedStats().Initiative,
			Entry.InitiativeRoll);
	}

	TurnQueue.Sort();
}

void UTurnManagerComponent::StartGlobalTurn()
{
	UE_LOG(LogTemp, Log, TEXT("=== Global Turn %d Started ==="), CurrentTurnNumber);
	OnGlobalTurnStarted.Broadcast(CurrentTurnNumber);
	ActivateNextUnit();
}

void UTurnManagerComponent::EndGlobalTurn()
{
	UE_LOG(LogTemp, Log, TEXT("=== Global Turn %d Ended ==="), CurrentTurnNumber);
	OnGlobalTurnEnded.Broadcast(CurrentTurnNumber);

	CurrentTurnNumber++;
	BuildInitiativeQueue();
	StartGlobalTurn();
}

void UTurnManagerComponent::ActivateNextUnit()
{
	if (TurnQueue.Num() == 0)
	{
		EndGlobalTurn();
		return;
	}

	if (BattleIsOver())
	{
		UBattleTeam* Winner = nullptr;

		if (AttackerTeam)
		{
			bool bAttackerAlive = false;
			for (AUnit* Unit : AttackerTeam->GetUnits())
			{
				if (Unit && Unit->GetCurrentHealth() > 0.0f)
				{
					bAttackerAlive = true;
					break;
				}
			}
			if (bAttackerAlive)
			{
				Winner = AttackerTeam;
			}
		}

		if (!Winner && DefenderTeam)
		{
			bool bDefenderAlive = false;
			for (AUnit* Unit : DefenderTeam->GetUnits())
			{
				if (Unit && Unit->GetCurrentHealth() > 0.0f)
				{
					bDefenderAlive = true;
					break;
				}
			}
			if (bDefenderAlive)
			{
				Winner = DefenderTeam;
			}
		}

		OnBattleEnded.Broadcast(Winner);
		EndBattle();
		return;
	}

	FTurnQueueEntry Entry = TurnQueue[0];
	TurnQueue.RemoveAt(0);

	ActiveUnit = Entry.Unit;
	ActiveUnitInitiative = Entry.TotalInitiative;

	UE_LOG(LogTemp, Log, TEXT(">>> %s's turn begins (Initiative: %d)"),
		*ActiveUnit->GetName(),
		Entry.TotalInitiative);

	OnUnitTurnStart.Broadcast(ActiveUnit);
	ActiveUnit->OnUnitTurnStart();
}

void UTurnManagerComponent::EndCurrentUnitTurn()
{
	if (!ActiveUnit)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("<<< %s's turn ends"), *ActiveUnit->GetName());

	ActiveUnit->OnUnitTurnEnd();
	OnUnitTurnEnd.Broadcast(ActiveUnit);

	ActiveUnit = nullptr;

	ActivateNextUnit();
}

void UTurnManagerComponent::InsertUnitIntoQueue(AUnit* Unit)
{
	if (!Unit)
	{
		return;
	}

	FTurnQueueEntry Entry;
	Entry.Unit = Unit;
	Entry.InitiativeRoll = RollInitiative();
	Entry.TotalInitiative = Unit->GetModifiedStats().Initiative + Entry.InitiativeRoll;

	// Find insertion point to maintain sort order
	int32 InsertIndex = 0;
	for (int32 i = 0; i < TurnQueue.Num(); ++i)
	{
		if (Entry.TotalInitiative <= TurnQueue[i].TotalInitiative)
		{
			InsertIndex = i + 1;
		}
		else
		{
			break;
		}
	}

	TurnQueue.Insert(Entry, InsertIndex);

	UE_LOG(LogTemp, Log, TEXT("Unit %s inserted into queue at position %d (Initiative: %d)"),
		*Unit->GetName(), InsertIndex, Entry.TotalInitiative);
}

void UTurnManagerComponent::RemoveUnitFromQueue(AUnit* Unit)
{
	if (!Unit)
	{
		return;
	}

	// Remove from queue
	for (int32 i = TurnQueue.Num() - 1; i >= 0; --i)
	{
		if (TurnQueue[i].Unit == Unit)
		{
			TurnQueue.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("Unit %s removed from turn queue"), *Unit->GetName());
		}
	}

	// If this was the active unit, end their turn
	if (Unit == ActiveUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Active unit %s died, advancing turn"), *Unit->GetName());
		ActiveUnit = nullptr;
		ActivateNextUnit();
	}
}

bool UTurnManagerComponent::CanUnitAct(AUnit* Unit) const
{
	return Unit == ActiveUnit && bBattleActive;
}

bool UTurnManagerComponent::BattleIsOver() const
{
	if (!AttackerTeam || !DefenderTeam)
	{
		return false;
	}

	bool bAttackerHasLivingUnits = false;
	bool bDefenderHasLivingUnits = false;

	for (AUnit* Unit : AttackerTeam->GetUnits())
	{
		if (Unit && Unit->GetCurrentHealth() > 0.0f)
		{
			bAttackerHasLivingUnits = true;
			break;
		}
	}

	for (AUnit* Unit : DefenderTeam->GetUnits())
	{
		if (Unit && Unit->GetCurrentHealth() > 0.0f)
		{
			bDefenderHasLivingUnits = true;
			break;
		}
	}

	return !bAttackerHasLivingUnits || !bDefenderHasLivingUnits;
}

void UTurnManagerComponent::HandleAbilityComplete(const FAbilityResult& Result)
{
	if (!Result.bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("TurnManager: Ability failed, not ending turn"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("TurnManager: Handling ability result with TurnAction: %d"),
		static_cast<uint8>(Result.TurnAction));

	switch (Result.TurnAction)
	{
	case EAbilityTurnAction::EndTurn:
		EndCurrentUnitTurn();
		break;

	case EAbilityTurnAction::FreeTurn:
		// Do nothing - unit keeps turn
		UE_LOG(LogTemp, Log, TEXT("TurnManager: Free turn - unit keeps acting"));
		break;

	case EAbilityTurnAction::EndTurnDelayed:
		// TODO: Set flag, end turn when animations finish
		UE_LOG(LogTemp, Warning, TEXT("TurnManager: EndTurnDelayed not fully implemented - ending turn now"));
		EndCurrentUnitTurn();
		break;

	case EAbilityTurnAction::RequireConfirm:
		// TODO: Set flag, wait for confirmation
		UE_LOG(LogTemp, Warning, TEXT("TurnManager: RequireConfirm not fully implemented - ending turn now"));
		EndCurrentUnitTurn();
		break;

	default:
		UE_LOG(LogTemp, Warning, TEXT("TurnManager: Unknown TurnAction, ending turn"));
		EndCurrentUnitTurn();
		break;
	}
}

int32 UTurnManagerComponent::RollInitiative() const
{
	return FMath::RandRange(InitiativeRollMin, InitiativeRollMax);
}

FUnitTurnQueueDisplay UTurnManagerComponent::MakeUnitTurnQueueDisplay(AUnit* Unit, int32 Initiative, bool bIsActiveUnit) const
{
	FUnitTurnQueueDisplay QueueDisplay;

	if (!Unit)
	{
		return QueueDisplay;
	}

	FUnitDisplayData UnitData = Unit->GetDisplayData();

	QueueDisplay.UnitName = UnitData.UnitName;
	QueueDisplay.CurrentInitiative = Initiative;
	QueueDisplay.PortraitTexture = UnitData.PortraitTexture;
	QueueDisplay.bIsActiveUnit = bIsActiveUnit;
	QueueDisplay.BelongsToAttackerTeam = AttackerTeam ? AttackerTeam->ContainsUnit(Unit) : false;

	return QueueDisplay;
}

TArray<FUnitTurnQueueDisplay> UTurnManagerComponent::GetQueueDisplayData() const
{
	TArray<FUnitTurnQueueDisplay> DisplayData;

	for (const FTurnQueueEntry& Entry : TurnQueue)
	{
		if (!Entry.Unit)
		{
			continue;
		}

		// Active unit is not in the queue, so bIsActive is always false
		FUnitTurnQueueDisplay QueueDisplay = MakeUnitTurnQueueDisplay(Entry.Unit, Entry.TotalInitiative, false);
		DisplayData.Add(QueueDisplay);
	}

	return DisplayData;
}

FUnitTurnQueueDisplay UTurnManagerComponent::GetActiveUnitDisplayData() const
{
	if (!ActiveUnit)
	{
		return FUnitTurnQueueDisplay();
	}
	return MakeUnitTurnQueueDisplay(ActiveUnit, ActiveUnitInitiative, true);
}
