#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
DEFINE_LOG_CATEGORY(LogKBSTurn);
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
#include "GameMechanics/Tactical/Grid/Subsystems/TacSubsystemControl.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacAICombatService.h"
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
		check(GridSubsys);
	}
	UTacCombatSubsystem* CombatSubsystem = GetWorld()->GetSubsystem<UTacCombatSubsystem>();
	checkf(CombatSubsystem, TEXT("UTacTurnSubsystem: CombatSubsystem not found"));
	AICombatService = NewObject<UTacAICombatService>(this);
	AICombatService->Initialize(GridSubsystem, CombatSubsystem);
	UTacSubsystemControl* Control = GetWorld()->GetSubsystem<UTacSubsystemControl>();
	Control->GridReadyForStart.AddDynamic(this, &UTacTurnSubsystem::GridAvailable);
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

void UTacTurnSubsystem::GridAvailable()
{
	UTacSubsystemControl* Control = GetWorld()->GetSubsystem<UTacSubsystemControl>();
	Control->NotifyTurnReady();
}

void UTacTurnSubsystem::InitializeStates()
{
	States.Add(ETurnState::EBattleInitializationState, MakeUnique<FBattleInitializationState>(this
		));
	States.Add(ETurnState::ERoundStartState, MakeUnique<FRoundStartState>(this));
	States.Add(ETurnState::ETurnStartState, MakeUnique<FTurnStartState>(this));
	States.Add(ETurnState::EActionsProcessingState, MakeUnique<FActionsProcessingState>(this));
	States.Add(ETurnState::ETurnEndState, MakeUnique<FTurnEndState>(this));
	States.Add(ETurnState::ERoundEndState, MakeUnique<FRoundEndState>(this));
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
	UE_LOG(LogKBSTurn, Log, TEXT("Battle started"));

	CurrentState = States[ETurnState::EBattleInitializationState].Get();
	CurrentState->Enter();

	AttemptTransition();
}

void UTacTurnSubsystem::TransitionToState(ETurnState NextState)
{
	check(CurrentState);

	if (auto NextStatePtr = States.Find(NextState))
	{
		const UEnum* StateEnum = StaticEnum<ETurnState>();
		UE_LOG(LogKBSTurn, Log, TEXT("State transition -> %s"), *StateEnum->GetNameStringByValue(static_cast<int64>(NextState)));
		CurrentState->Exit();
		CurrentState = NextStatePtr->Get();
		CurrentState->Enter();
	}
	else
	{
		UE_LOG(LogKBSTurn, Error, TEXT("Invalid next state requested: %d"), static_cast<int32>(NextState));
	}
}

void UTacTurnSubsystem::AttemptTransition()
{
	AttemptTransition(0);
}

void UTacTurnSubsystem::AttemptTransition(int32 Depth)
{
	checkf(Depth < 100, TEXT("TacTurnSubsystem: AttemptTransition exceeded 100 automatic transitions - likely infinite loop in state machine"));
	check(CurrentState);

	// Check win condition - if battle ended, force to battle end state
	if (CurrentState->CheckWinCondition())
	{
		TransitionToState(ETurnState::EBattleEndState);
		return;
	}

	ETurnProcessingSubstate Substate = CurrentState->CanReleaseState();

	// Can't release - stuck in awaiting input or presentation
	if (Substate != ETurnProcessingSubstate::EFreeState)
	{
		return;
	}

	// Free to transition - do ONE transition
	ETurnState NextStateEnum = CurrentState->NextState();
	TransitionToState(NextStateEnum);

	// Recursively attempt one more transition
	// (allows rapid transitions through multiple "instant" states without waiting for next event)
	AttemptTransition(Depth + 1);
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
		UE_LOG(LogKBSTurn, Log, TEXT("Wait() called"));
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

void UTacTurnSubsystem::HandleUnitDied(AUnit* Unit)
{
	TurnOrder->RemoveUnit(Unit);
}

void UTacTurnSubsystem::RegisterSummonedUnit(AUnit* Unit)
{
	checkf(Unit, TEXT("RegisterSummonedUnit: null unit"));
	Unit->OnUnitDied.AddDynamic(this, &UTacTurnSubsystem::HandleUnitDied);
	TurnOrder->InsertUnit(Unit);
}

void UTacTurnSubsystem::ReloadTurnOrder()
{
	TArray<AUnit*> Units = GridSubsystem->GetAllAliveUnits();
	Units.RemoveAll([](AUnit* Unit) { return Unit->GetStats().Status.IsFleeing(); });
	TurnOrder->Repopulate(Units, GridSubsystem->GetAttackerTeam());
	for (AUnit* Unit : Units)
	{
		Unit->OnUnitDied.RemoveDynamic(this, &UTacTurnSubsystem::HandleUnitDied);
		Unit->OnUnitDied.AddDynamic(this, &UTacTurnSubsystem::HandleUnitDied);
	}
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
