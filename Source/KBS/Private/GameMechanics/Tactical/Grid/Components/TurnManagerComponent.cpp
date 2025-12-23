#include "GameMechanics/Tactical/Grid/Components/TurnManagerComponent.h"
#include "GameMechanics/Tactical/Grid/Components/PresentationTrackerComponent.h"
#include "GameMechanics/Tactical/Grid/Components/AbilityExecutorComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridHighlightComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridTargetingComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridMovementComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridInputLockComponent.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Units/UnitStats.h"
#include "GameplayTypes/AbilityTypes.h"
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
	if (InputLockComponent)
	{
		InputLockComponent->ReleaseLock(EInputLockSource::TurnTransition);
	}
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
	for (int32 i = TurnQueue.Num() - 1; i >= 0; --i)
	{
		if (TurnQueue[i].Unit == Unit)
		{
			TurnQueue.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("Unit %s removed from turn queue"), *Unit->GetName());
		}
	}
	if (Unit == ActiveUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Active unit %s died, advancing turn"), *Unit->GetName());
		ActiveUnit = nullptr;
		ActivateNextUnit();
	}
}
void UTurnManagerComponent::WaitCurrentUnit()
{
	if (!ActiveUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaitCurrentUnit called but no active unit"));
		return;
	}
	int32 NewInitiative = -ActiveUnitInitiative;
	UE_LOG(LogTemp, Log, TEXT("%s waiting - Initiative negated from %d to %d"),
		*ActiveUnit->GetName(), ActiveUnitInitiative, NewInitiative);
	FTurnQueueEntry Entry;
	Entry.Unit = ActiveUnit;
	Entry.InitiativeRoll = 0;
	Entry.TotalInitiative = NewInitiative;
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
	UE_LOG(LogTemp, Log, TEXT("Unit reinserted at queue position %d"), InsertIndex);
	ActiveUnit = nullptr;
	ActivateNextUnit();
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
	case EAbilityTurnAction::EndTurnDelayed:
		if (PresentationTracker && !PresentationTracker->IsIdle())
		{
			UE_LOG(LogTemp, Log, TEXT("TurnManager: Waiting for presentation to complete before ending turn"));
			bWaitingForPresentation = true;
			PresentationTracker->OnAllOperationsComplete.AddDynamic(this, &UTurnManagerComponent::OnPresentationComplete);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("TurnManager: No pending presentation, ending turn immediately"));
			if (InputLockComponent)
			{
				InputLockComponent->ReleaseLock(EInputLockSource::AbilityExecution);
			}
			EndCurrentUnitTurn();
		}
		break;
	case EAbilityTurnAction::FreeTurn:
		UE_LOG(LogTemp, Log, TEXT("TurnManager: Free turn - unit keeps acting"));
		if (InputLockComponent)
		{
			InputLockComponent->ReleaseLock(EInputLockSource::AbilityExecution);
		}
		break;
	case EAbilityTurnAction::RequireConfirm:
		UE_LOG(LogTemp, Warning, TEXT("TurnManager: RequireConfirm not fully implemented - ending turn now"));
		if (InputLockComponent)
		{
			InputLockComponent->ReleaseLock(EInputLockSource::AbilityExecution);
		}
		EndCurrentUnitTurn();
		break;
	case EAbilityTurnAction::Wait:
		UE_LOG(LogTemp, Log, TEXT("TurnManager: Unit waiting - reinserting with negated initiative"));
		if (InputLockComponent)
		{
			InputLockComponent->ReleaseLock(EInputLockSource::AbilityExecution);
		}
		WaitCurrentUnit();
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("TurnManager: Unknown TurnAction, ending turn"));
		if (InputLockComponent)
		{
			InputLockComponent->ReleaseLock(EInputLockSource::AbilityExecution);
		}
		EndCurrentUnitTurn();
		break;
	}
}
void UTurnManagerComponent::SwitchAbility(UUnitAbilityInstance* NewAbility)
{
	if (!NewAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchAbility: NewAbility is null"));
		return;
	}
	if (!ActiveUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchAbility: No active unit"));
		return;
	}
	if (!ActiveUnit->AbilityInventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchAbility: Active unit has no AbilityInventory"));
		return;
	}
	const TArray<UUnitAbilityInstance*>& AvailableAbilities = ActiveUnit->AbilityInventory->GetAvailableActiveAbilities();
	if (!AvailableAbilities.Contains(NewAbility))
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchAbility: Ability not in unit's inventory"));
		return;
	}
	ActiveUnit->AbilityInventory->EquipAbility(NewAbility);
	UE_LOG(LogTemp, Log, TEXT("SwitchAbility: Equipped %s on %s"),
		*NewAbility->GetAbilityDisplayData().AbilityName, *ActiveUnit->GetName());
	if (HighlightComponent)
	{
		HighlightComponent->ClearHighlights();
		if (TargetingComponent && MovementComponent && HighlightComponent)
		{
			const TArray<FIntPoint> TargetCells = TargetingComponent->GetValidTargetCells(ActiveUnit);
			HighlightComponent->ShowValidTargets(TargetCells);
			const TArray<FIntPoint> ValidCells = MovementComponent->GetValidMoveCells(ActiveUnit);
			HighlightComponent->ShowValidMoves(ValidCells);
		}
	}
	ETargetReach Targeting = NewAbility->GetTargeting();
	if (Targeting == ETargetReach::Self)
	{
		UE_LOG(LogTemp, Log, TEXT("SwitchAbility: Self-targeting ability, executing immediately"));
		ExecuteAbilityOnSelf(ActiveUnit, NewAbility);
	}
}
void UTurnManagerComponent::ExecuteAbilityOnTargets(AUnit* SourceUnit, const TArray<AUnit*>& Targets)
{
	if (!SourceUnit || Targets.Num() == 0 || !AbilityExecutor)
	{
		return;
	}
	if (!SourceUnit->AbilityInventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteAbilityOnTargets: Source unit has no AbilityInventory"));
		return;
	}
	UUnitAbilityInstance* CurrentAbility = SourceUnit->AbilityInventory->GetCurrentActiveAbility();
	if (!CurrentAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteAbilityOnTargets: Source unit has no current active ability"));
		return;
	}
	if (InputLockComponent)
	{
		InputLockComponent->RequestLock(EInputLockSource::AbilityExecution);
	}
	FAbilityBattleContext Context = AbilityExecutor->BuildContext(SourceUnit, Targets);
	FAbilityResult Result = AbilityExecutor->ExecuteAbility(CurrentAbility, Context);
	AbilityExecutor->ResolveResult(Result);
	HandleAbilityComplete(Result);
}
void UTurnManagerComponent::ExecuteAbilityOnSelf(AUnit* SourceUnit, UUnitAbilityInstance* Ability)
{
	if (!SourceUnit || !Ability || !AbilityExecutor)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteAbilityOnSelf: Invalid source unit or ability"));
		return;
	}
	if (InputLockComponent)
	{
		InputLockComponent->RequestLock(EInputLockSource::AbilityExecution);
	}
	TArray<AUnit*> SelfTarget;
	SelfTarget.Add(SourceUnit);
	FAbilityBattleContext Context = AbilityExecutor->BuildContext(SourceUnit, SelfTarget);
	FAbilityValidation Validation = AbilityExecutor->ValidateAbility(Ability, Context);
	if (!Validation.bIsValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteAbilityOnSelf: Validation failed - %s"),
			*Validation.FailureMessage.ToString());
		return;
	}
	FAbilityResult Result = AbilityExecutor->ExecuteAbility(Ability, Context);
	AbilityExecutor->ResolveResult(Result);
	HandleAbilityComplete(Result);
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
void UTurnManagerComponent::OnPresentationComplete()
{
	UE_LOG(LogTemp, Log, TEXT("TurnManager: Presentation complete, ending turn"));
	bWaitingForPresentation = false;
	if (PresentationTracker)
	{
		PresentationTracker->OnAllOperationsComplete.RemoveDynamic(this, &UTurnManagerComponent::OnPresentationComplete);
	}
	if (InputLockComponent)
	{
		InputLockComponent->ReleaseLock(EInputLockSource::AbilityExecution);
	}
	EndCurrentUnitTurn();
}
