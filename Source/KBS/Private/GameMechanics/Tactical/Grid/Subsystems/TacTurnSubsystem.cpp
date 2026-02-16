#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/TacTurnOrder.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/BattleInitializationState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/RoundStartState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/TurnStartState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/ActionsProcessingState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/TurnEndState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/RoundEndState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/BattleEndState.h"
#include "GameMechanics/Tactical/PresentationSubsystem.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/GridCoordinates.h"

void UTacTurnSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	if (UPresentationSubsystem* PresentationSys = GetWorld()->GetSubsystem<UPresentationSubsystem>())
	{
		PresentationSys->OnAllPresentationsComplete.AddDynamic(this, &UTacTurnSubsystem::OnPresentationComplete);
	}
	if (UTacGridSubsystem* GridSubsys = GetWorld()->GetSubsystem<UTacGridSubsystem>())
	{
		GridSubsystem = GridSubsys;
		GridSubsystem->Ready.AddDynamic(this, &UTacTurnSubsystem::GridAvailable);
	}
}

void UTacTurnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TurnOrder = MakeUnique<FTacTurnOrder>();
	InitializeStates();
}

void UTacTurnSubsystem::Deinitialize()
{
	// Unsubscribe from presentation delegate
	if (UPresentationSubsystem* PresentationSys = GetWorld()->GetSubsystem<UPresentationSubsystem>())
	{
		PresentationSys->OnAllPresentationsComplete.RemoveDynamic(this, &UTacTurnSubsystem::OnPresentationComplete);
	}

	if (CurrentState)
	{
		CurrentState->Exit();
		CurrentState = nullptr;
	}

	States.Empty();
	TurnOrder.Reset();

	Super::Deinitialize();
}

void UTacTurnSubsystem::GridAvailable(UTacGridSubsystem* Grid)
{
}

void UTacTurnSubsystem::InitializeStates()
{
	States.Add(ETurnState::EBattleInitializationState, MakeUnique<FBattleInitializationState>(this
		));
	States.Add(ETurnState::ERoundStartState, MakeUnique<FRoundStartState>(this));
	States.Add(ETurnState::ETurnStartState, MakeUnique<FTurnStartState>(this));
	States.Add(ETurnState::EActionsProcessingState, MakeUnique<FActionsProcessingState>(this));
	States.Add(ETurnState::ETurnEndState, MakeUnique<FTurnEndState>(this));
	States.Add(ETurnState::EBattleEndState, MakeUnique<FBattleEndState>(this));

	// Set parent reference for all states
	for (auto& Pair : States)
	{
		Pair.Value->ParentTurnSubsystem = this;
	}
}

void UTacTurnSubsystem::StartBattle()
{
	checkf(States.Num() > 0, TEXT("TacTurnSubsystem: States not initialized"));

	CurrentState = States[ETurnState::EBattleInitializationState].Get();
	CurrentState->Enter();

	AttemptTransition();
}

void UTacTurnSubsystem::TransitionToState(ETurnState NextState)
{
	check(CurrentState);

	if (auto NextStatePtr = States.Find(NextState))
	{
		CurrentState->Exit();
		CurrentState = NextStatePtr->Get();
		CurrentState->Enter();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TacTurnSubsystem: Invalid next state"));
	}
}

void UTacTurnSubsystem::AttemptTransition()
{
	check(CurrentState);

	// Transition loop: keep transitioning until stuck in awaiting input or presentation
	while (true)
	{
		// Check win condition - if battle ended, force to battle end state
		if (CurrentState->CheckWinCondition())
		{
			TransitionToState(ETurnState::EBattleEndState);
			break;
		}

		ETurnProcessingSubstate Substate = CurrentState->CanReleaseState();

		// Can't release - stuck in awaiting input
		if (Substate == ETurnProcessingSubstate::EAwaitingInputState)
		{
			break;
		}

		// Awaiting presentation - check if presentation is actually running
		if (Substate == ETurnProcessingSubstate::EAwaitingPresentationState)
		{
			break;
		}

		// Free to transition
		if (Substate == ETurnProcessingSubstate::EFreeState)
		{
			ETurnState NextStateEnum = CurrentState->NextState();
			TransitionToState(NextStateEnum);
		}
	}
}

void UTacTurnSubsystem::UnitClicked(AUnit* Unit)
{
	check(CurrentState);
	CurrentState->UnitClicked(Unit);
	AttemptTransition();
}

void UTacTurnSubsystem::CellClicked(FTacCoordinates Cell)
{
	check(CurrentState);
	CurrentState->CellClicked(Cell);
	AttemptTransition();
}

void UTacTurnSubsystem::AbilityClicked(UUnitAbilityInstance* Ability)
{
	check(CurrentState);
	CurrentState->AbilityClicked(Ability);
	AttemptTransition();
}

void UTacTurnSubsystem::OnPresentationComplete()
{
	check(CurrentState);
	CurrentState->OnPresentationComplete();
	AttemptTransition();
}

void UTacTurnSubsystem::Wait()
{
	if (TurnOrder)
	{
		TurnOrder->Wait();
	}
}

AUnit* UTacTurnSubsystem::GetCurrentUnit() const
{
	return TurnOrder ? TurnOrder->GetCurrentUnit() : nullptr;
}

TArray<AUnit*> UTacTurnSubsystem::GetRemainingUnits(int32 TruncList) const
{
	return TurnOrder ? TurnOrder->GetRemainingUnits(TruncList) : TArray<AUnit*>();
}

int32 UTacTurnSubsystem::GetUnitInitiative(AUnit* Unit) const
{
	return TurnOrder ? TurnOrder->GetUnitInitiative(Unit) : 0;
}

void UTacTurnSubsystem::ReloadTurnOrder()
{
	TurnOrder->Repopulate(GridSubsystem->GetActiveUnits(), GridSubsystem->GetAttackerTeam());
}

void UTacTurnSubsystem::BroadcastRoundStart()
{
	OnRoundStart.Broadcast(CurrentRound);
}

void UTacTurnSubsystem::BroadcastRoundEnd()
{
	OnRoundEnd.Broadcast(CurrentRound);
}

void UTacTurnSubsystem::BroadcastTurnStart()
{
	OnTurnStart.Broadcast(TurnOrder->GetCurrentUnit());
}

void UTacTurnSubsystem::BroadcastTurnEnd()
{
	OnTurnEnd.Broadcast(TurnOrder->GetCurrentUnit());
}

void UTacTurnSubsystem::BroadcastBattleEnd()
{
	OnBattleEnd.Broadcast(GridSubsystem->GetWinnerTeam());
}
