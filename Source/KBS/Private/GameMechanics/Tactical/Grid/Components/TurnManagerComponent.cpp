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

int32 UTurnManagerComponent::RollInitiative() const
{
	return FMath::RandRange(InitiativeRollMin, InitiativeRollMax);
}
