#include "GameMechanics/Tactical/Grid/Components/TurnManagerComponent.h"
#include "GameMechanics/Tactical/PresentationSubsystem.h"
#include "GameMechanics/Tactical/Grid/Components/AbilityExecutorComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridHighlightComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridTargetingComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridMovementComponent.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Units/UnitStats.h"
#include "GameplayTypes/AbilityTypes.h"
UTurnManagerComponent::UTurnManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// State Machine initialization
	CurrentState = EBattleState::Idle;
	PlayerSubstate = EPlayerTurnSubstate::None;
	EnemySubstate = EEnemyTurnSubstate::None;
	StateEnterTime = 0.0f;
	MaxStateTimeout = 30.0f;
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
		bool bHasWinner = false;
		ETeamSide WinningSide = ETeamSide::Attacker;

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
				bHasWinner = true;
				WinningSide = AttackerTeam->GetTeamSide();
			}
		}
		if (!bHasWinner && DefenderTeam)
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
				bHasWinner = true;
				WinningSide = DefenderTeam->GetTeamSide();
			}
		}
		OnBattleEnded.Broadcast(bHasWinner, WinningSide);
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

	// State transition based on team affiliation
	bool bIsPlayerControlled = (ActiveUnit->GetTeamSide() == PlayerControlledTeam);

	if (bIsPlayerControlled)
	{
		TransitionToState(EBattleState::PlayerTurn, EPlayerTurnSubstate::AwaitingInput, EEnemyTurnSubstate::None);
	}
	else
	{
		TransitionToState(EBattleState::EnemyTurn, EPlayerTurnSubstate::None, EEnemyTurnSubstate::Thinking);
	}

	// Call unit's turn start BEFORE broadcasting (so DOT ticks before AI acts)
	ActiveUnit->OnUnitTurnStart();
	OnUnitTurnStart.Broadcast(ActiveUnit);
}
void UTurnManagerComponent::EndCurrentUnitTurn()
{
	if (!ActiveUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("EndCurrentUnitTurn called but ActiveUnit is null - likely already ended"));
		// If already waiting for presentation, let it complete naturally
		if (!bWaitingForPresentation)
		{
			ActivateNextUnit();
		}
		return;
	}

	AUnit* UnitEndingTurn = ActiveUnit;
	UE_LOG(LogTemp, Log, TEXT("<<< %s's turn ends"), *UnitEndingTurn->GetName());
	UnitEndingTurn->OnUnitTurnEnd();
	OnUnitTurnEnd.Broadcast(UnitEndingTurn);
	ActiveUnit = nullptr;

	// Wait for any pending presentations before starting next turn
	UPresentationSubsystem* PresentationSys = UPresentationSubsystem::Get(this);
	if (PresentationSys && !PresentationSys->IsIdle())
	{
		UE_LOG(LogTemp, Log, TEXT("TurnManager: Waiting for presentation to complete before starting next turn"));
		if (!bWaitingForPresentation)
		{
			bWaitingForPresentation = true;
			PresentationSys->OnAllPresentationsComplete.AddDynamic(this, &UTurnManagerComponent::OnPresentationComplete);
		}
	}
	else
	{
		ActivateNextUnit();
	}
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

		// Only activate next unit if we're not waiting for presentation to complete
		// If waiting, OnPresentationComplete will handle activation
		if (!bWaitingForPresentation)
		{
			ActivateNextUnit();
		}
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
		if (HighlightComponent)
		{
			HighlightComponent->ClearHighlights();
		}

		// Revert to awaiting input on failure
		if (CurrentState == EBattleState::PlayerTurn)
		{
			TransitionToState(EBattleState::PlayerTurn, EPlayerTurnSubstate::AwaitingInput, EEnemyTurnSubstate::None);
		}
		else if (CurrentState == EBattleState::EnemyTurn)
		{
			TransitionToState(EBattleState::EnemyTurn, EPlayerTurnSubstate::None, EEnemyTurnSubstate::Thinking);
		}
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("TurnManager: Handling ability result with TurnAction: %d"),
		static_cast<uint8>(Result.TurnAction));

	if (HighlightComponent)
	{
		HighlightComponent->ClearHighlights();
	}

	switch (Result.TurnAction)
	{
	case EAbilityTurnAction::EndTurn:
	case EAbilityTurnAction::EndTurnDelayed:
		{
			UPresentationSubsystem* PresentationSys = UPresentationSubsystem::Get(this);
			if (PresentationSys && !PresentationSys->IsIdle())
			{
				UE_LOG(LogTemp, Log, TEXT("TurnManager: Waiting for presentation to complete before ending turn"));

				// Transition to PlayingPresentation
				if (CurrentState == EBattleState::PlayerTurn)
				{
					TransitionToState(EBattleState::PlayerTurn, EPlayerTurnSubstate::PlayingPresentation, EEnemyTurnSubstate::None);
				}
				else if (CurrentState == EBattleState::EnemyTurn)
				{
					TransitionToState(EBattleState::EnemyTurn, EPlayerTurnSubstate::None, EEnemyTurnSubstate::PlayingPresentation);
				}

				if (!bWaitingForPresentation)
				{
					bWaitingForPresentation = true;
					PresentationSys->OnAllPresentationsComplete.AddDynamic(this, &UTurnManagerComponent::OnPresentationComplete);
				}
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("TurnManager: No pending presentation, ending turn immediately"));
				TransitionToState(EBattleState::TurnTransition, EPlayerTurnSubstate::None, EEnemyTurnSubstate::None);
				EndCurrentUnitTurn();
			}
		}
		break;
	case EAbilityTurnAction::FreeTurn:
		UE_LOG(LogTemp, Log, TEXT("TurnManager: Free turn - unit keeps acting"));

		// Return to AwaitingInput for free turn
		if (CurrentState == EBattleState::PlayerTurn)
		{
			TransitionToState(EBattleState::PlayerTurn, EPlayerTurnSubstate::AwaitingInput, EEnemyTurnSubstate::None);
		}
		else if (CurrentState == EBattleState::EnemyTurn)
		{
			TransitionToState(EBattleState::EnemyTurn, EPlayerTurnSubstate::None, EEnemyTurnSubstate::Thinking);
		}
		break;
	case EAbilityTurnAction::RequireConfirm:
		UE_LOG(LogTemp, Warning, TEXT("TurnManager: RequireConfirm not fully implemented - ending turn now"));
		TransitionToState(EBattleState::TurnTransition, EPlayerTurnSubstate::None, EEnemyTurnSubstate::None);
		EndCurrentUnitTurn();
		break;
	case EAbilityTurnAction::Wait:
		UE_LOG(LogTemp, Log, TEXT("TurnManager: Unit waiting - reinserting with negated initiative"));
		TransitionToState(EBattleState::TurnTransition, EPlayerTurnSubstate::None, EEnemyTurnSubstate::None);
		WaitCurrentUnit();
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("TurnManager: Unknown TurnAction, ending turn"));
		TransitionToState(EBattleState::TurnTransition, EPlayerTurnSubstate::None, EEnemyTurnSubstate::None);
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
	ETargetReach Targeting = NewAbility->GetTargeting();
	if (HighlightComponent)
	{
		HighlightComponent->ClearHighlights();
		if (TargetingComponent)
		{
			const TArray<FIntPoint> TargetCells = TargetingComponent->GetValidTargetCells(ActiveUnit);
			HighlightComponent->ShowHighlightsForTargeting(TargetCells, Targeting);
		}
	}
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

	// State transition to ProcessingAbility or ExecutingAction
	if (CurrentState == EBattleState::PlayerTurn)
	{
		TransitionToState(EBattleState::PlayerTurn, EPlayerTurnSubstate::ProcessingAbility, EEnemyTurnSubstate::None);
	}
	else if (CurrentState == EBattleState::EnemyTurn)
	{
		TransitionToState(EBattleState::EnemyTurn, EPlayerTurnSubstate::None, EEnemyTurnSubstate::ExecutingAction);
	}

	FAbilityBattleContext Context = AbilityExecutor->BuildContext(SourceUnit, Targets);
	FAbilityResult Result = AbilityExecutor->ExecuteAbility(CurrentAbility, Context);
	AbilityExecutor->ResolveResult(Result);
	HandleAbilityComplete(Result);
}

void UTurnManagerComponent::ExecuteAbilityOnTargets(AUnit* SourceUnit, const TArray<AUnit*>& Targets, FIntPoint ClickedCell, uint8 ClickedLayer)
{
	if (!SourceUnit || !AbilityExecutor)
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

	// State transition to ProcessingAbility or ExecutingAction
	if (CurrentState == EBattleState::PlayerTurn)
	{
		TransitionToState(EBattleState::PlayerTurn, EPlayerTurnSubstate::ProcessingAbility, EEnemyTurnSubstate::None);
	}
	else if (CurrentState == EBattleState::EnemyTurn)
	{
		TransitionToState(EBattleState::EnemyTurn, EPlayerTurnSubstate::None, EEnemyTurnSubstate::ExecutingAction);
	}

	FAbilityBattleContext Context = AbilityExecutor->BuildContext(SourceUnit, Targets, ClickedCell, ClickedLayer);
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

	// State transition to ProcessingAbility or ExecutingAction
	if (CurrentState == EBattleState::PlayerTurn)
	{
		TransitionToState(EBattleState::PlayerTurn, EPlayerTurnSubstate::ProcessingAbility, EEnemyTurnSubstate::None);
	}
	else if (CurrentState == EBattleState::EnemyTurn)
	{
		TransitionToState(EBattleState::EnemyTurn, EPlayerTurnSubstate::None, EEnemyTurnSubstate::ExecutingAction);
	}

	TArray<AUnit*> SelfTarget;
	SelfTarget.Add(SourceUnit);
	FAbilityBattleContext Context = AbilityExecutor->BuildContext(SourceUnit, SelfTarget);
	FAbilityValidation Validation = AbilityExecutor->ValidateAbility(Ability, Context);
	if (!Validation.bIsValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteAbilityOnSelf: Validation failed - %s"),
			*Validation.FailureMessage.ToString());

		// Revert state on validation failure
		if (CurrentState == EBattleState::PlayerTurn)
		{
			TransitionToState(EBattleState::PlayerTurn, EPlayerTurnSubstate::AwaitingInput, EEnemyTurnSubstate::None);
		}
		else if (CurrentState == EBattleState::EnemyTurn)
		{
			TransitionToState(EBattleState::EnemyTurn, EPlayerTurnSubstate::None, EEnemyTurnSubstate::Thinking);
		}
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
	bWaitingForPresentation = false;
	UPresentationSubsystem* PresentationSys = UPresentationSubsystem::Get(this);
	if (PresentationSys)
	{
		PresentationSys->OnAllPresentationsComplete.RemoveDynamic(this, &UTurnManagerComponent::OnPresentationComplete);
	}

	// Transition out of presentation state
	TransitionToState(EBattleState::TurnTransition, EPlayerTurnSubstate::None, EEnemyTurnSubstate::None);

	// Check if we're waiting after ability execution or after turn end
	if (ActiveUnit)
	{
		// We're still in a turn, end it now
		UE_LOG(LogTemp, Log, TEXT("TurnManager: Presentation complete, ending turn"));
		EndCurrentUnitTurn();
	}
	else
	{
		// Turn already ended, just activate next unit
		UE_LOG(LogTemp, Log, TEXT("TurnManager: Presentation complete, starting next turn"));
		ActivateNextUnit();
	}
}

// State Machine Implementation
void UTurnManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetWorld())
	{
		return;
	}

	float TimeInState = GetWorld()->GetTimeSeconds() - StateEnterTime;
	if (TimeInState > MaxStateTimeout && CurrentState != EBattleState::Idle)
	{
		UE_LOG(LogTemp, Error, TEXT("[STATE] Stuck in %s for %.1fs - MANUAL RECOVERY REQUIRED"),
			*GetCurrentStateName(), TimeInState);
	}
}

bool UTurnManagerComponent::CanAcceptInput() const
{
	if (CurrentState != EBattleState::PlayerTurn)
	{
		return false;
	}
	return PlayerSubstate == EPlayerTurnSubstate::AwaitingInput;
}

void UTurnManagerComponent::TransitionToState(EBattleState NewState, EPlayerTurnSubstate NewPlayerSubstate, EEnemyTurnSubstate NewEnemySubstate)
{
	if (!IsValidTransition(NewState, NewPlayerSubstate, NewEnemySubstate))
	{
		UE_LOG(LogTemp, Error, TEXT("[STATE] Invalid transition from %s to %s"),
			*GetCurrentStateName(), *GetStateName(NewState, NewPlayerSubstate, NewEnemySubstate));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[STATE] %s → %s"),
		*GetCurrentStateName(), *GetStateName(NewState, NewPlayerSubstate, NewEnemySubstate));

	CurrentState = NewState;
	PlayerSubstate = NewPlayerSubstate;
	EnemySubstate = NewEnemySubstate;

	if (GetWorld())
	{
		StateEnterTime = GetWorld()->GetTimeSeconds();
	}

	OnBattleStateChanged.Broadcast(CurrentState, PlayerSubstate, EnemySubstate);
}

bool UTurnManagerComponent::IsValidTransition(EBattleState NewState, EPlayerTurnSubstate NewPlayerSubstate, EEnemyTurnSubstate NewEnemySubstate) const
{
	// Basic validation: PlayerTurn should have PlayerSubstate, not EnemySubstate
	if (NewState == EBattleState::PlayerTurn && NewPlayerSubstate == EPlayerTurnSubstate::None)
	{
		return false;
	}
	if (NewState == EBattleState::EnemyTurn && NewEnemySubstate == EEnemyTurnSubstate::None)
	{
		return false;
	}
	// Other states should have None substates
	if ((NewState == EBattleState::Idle || NewState == EBattleState::TurnTransition || NewState == EBattleState::RoundTransition) &&
		(NewPlayerSubstate != EPlayerTurnSubstate::None || NewEnemySubstate != EEnemyTurnSubstate::None))
	{
		return false;
	}
	return true;
}

FString UTurnManagerComponent::GetCurrentStateName() const
{
	return GetStateName(CurrentState, PlayerSubstate, EnemySubstate);
}

FString UTurnManagerComponent::GetStateName(EBattleState State, EPlayerTurnSubstate PlayerSub, EEnemyTurnSubstate EnemySub) const
{
	FString StateName;
	switch (State)
	{
	case EBattleState::Idle:
		StateName = TEXT("Idle");
		break;
	case EBattleState::PlayerTurn:
		StateName = TEXT("PlayerTurn:");
		switch (PlayerSub)
		{
		case EPlayerTurnSubstate::AwaitingInput:
			StateName += TEXT("AwaitingInput");
			break;
		case EPlayerTurnSubstate::ProcessingAbility:
			StateName += TEXT("ProcessingAbility");
			break;
		case EPlayerTurnSubstate::PlayingPresentation:
			StateName += TEXT("PlayingPresentation");
			break;
		default:
			StateName += TEXT("None");
			break;
		}
		break;
	case EBattleState::EnemyTurn:
		StateName = TEXT("EnemyTurn:");
		switch (EnemySub)
		{
		case EEnemyTurnSubstate::Thinking:
			StateName += TEXT("Thinking");
			break;
		case EEnemyTurnSubstate::ExecutingAction:
			StateName += TEXT("ExecutingAction");
			break;
		case EEnemyTurnSubstate::PlayingPresentation:
			StateName += TEXT("PlayingPresentation");
			break;
		default:
			StateName += TEXT("None");
			break;
		}
		break;
	case EBattleState::TurnTransition:
		StateName = TEXT("TurnTransition");
		break;
	case EBattleState::RoundTransition:
		StateName = TEXT("RoundTransition");
		break;
	default:
		StateName = TEXT("Unknown");
		break;
	}
	return StateName;
}
