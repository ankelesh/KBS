#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
DEFINE_LOG_CATEGORY(LogKBSTurn);
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/TacTurnOrder.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/BattleInitializationState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/RoundStartState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/TurnStartState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/ActionsProcessingState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/TurnEndState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/RoundEndState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/BattleEndState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacSubsystemControl.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacAICombatService.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameplayTypes/GridCoordinates.h"
#include "Presentation/Tactical/TacticalPresentationBuilder.h"
#include "Presentation/Core/PresentationSequencePlayer.h"
#include "Presentation/Core/PresentationSequence.h"

void UTacTurnSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
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

	PresentationBuilder = NewObject<UTacticalPresentationBuilder>(this);
	PresentationBuilder->SetGridSubsystem(GridSubsystem);
	PresentationBuilder->SetLogSubsystem(GetWorld()->GetSubsystem<UTacLogSubsystem>());

	if (UPresentationSequencePlayer* Player = UPresentationSequencePlayer::Get(this))
	{
		Player->OnPresentationComplete.AddDynamic(this, &UTacTurnSubsystem::OnPresentationComplete);
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
	if (UPresentationSequencePlayer* Player = UPresentationSequencePlayer::Get(this))
	{
		Player->OnPresentationComplete.RemoveDynamic(this, &UTacTurnSubsystem::OnPresentationComplete);
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
	// DataManager (and thus the grid's config) only exists once RegisterManager has run - safe from here on.
	PresentationBuilder->SetConfig(GridSubsystem->GetPresentationConfig());

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

	CurrentStateEnum = ETurnState::EBattleInitializationState;
	CurrentState = States[ETurnState::EBattleInitializationState].Get();
	CurrentState->Enter();
	RecordPhase(ETurnState::EBattleInitializationState);

	PendingTrigger = TEXT("BattleStart");
	AttemptTransition();
}

void UTacTurnSubsystem::TransitionToState(ETurnState NextState)
{
	check(CurrentState);

	if (auto NextStatePtr = States.Find(NextState))
	{
		const UEnum* StateEnum = StaticEnum<ETurnState>();
		UE_LOG(LogKBSTurn, Log, TEXT("State transition -> %s"), *StateEnum->GetNameStringByValue(static_cast<int64>(NextState)));
		RecordTransition(CurrentStateEnum, NextState);
		CurrentState->Exit();
		CurrentStateEnum = NextState;
		CurrentState = NextStatePtr->Get();
		CurrentState->Enter();
		RecordPhase(NextState);
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
	if (bAwaitingPresentation)
	{
		return;
	}

	if (Depth >= 100)
	{
		DumpInfiniteLoopDiagnostics();
		TransitionToState(ETurnState::EBattleEndState);
		return;
	}
	check(CurrentState);

	// Check win condition - if battle ended, present the final slice (so despawns finalize) then
	// force to battle end state.
	if (CurrentState->CheckWinCondition())
	{
		if (PresentPendingSlice())
		{
			bAwaitingPresentation = true;
			return;
		}
		TransitionToState(ETurnState::EBattleEndState);
		return;
	}

	ETurnProcessingSubstate Substate = CurrentState->CanReleaseState();

	// Can't release - stuck in awaiting input or presentation
	if (Substate != ETurnProcessingSubstate::EFreeState)
	{
		return;
	}

	// Free to transition - present whatever the state's Enter() appended, then do ONE transition
	if (PresentPendingSlice())
	{
		bAwaitingPresentation = true;
		return;
	}

	ETurnState NextStateEnum = CurrentState->NextState();
	TransitionToState(NextStateEnum);

	// Recursively attempt one more transition
	// (allows rapid transitions through multiple "instant" states without waiting for next event)
	AttemptTransition(Depth + 1);
}

void UTacTurnSubsystem::UnitClicked(AUnit* Unit)
{
	check(CurrentState);
	RecordEvent(TEXT("UnitClicked"));
	PendingTrigger = TEXT("UnitClicked");
	CurrentState->UnitClicked(Unit);
	AttemptTransition();
}

void UTacTurnSubsystem::CellClicked(FTacCoordinates Cell)
{
	check(CurrentState);
	RecordEvent(TEXT("CellClicked"));
	PendingTrigger = TEXT("CellClicked");
	CurrentState->CellClicked(Cell);
	AttemptTransition();
}

void UTacTurnSubsystem::AbilityClicked(UUnitAbility* Ability)
{
	check(CurrentState);
	RecordEvent(TEXT("AbilityClicked"));
	PendingTrigger = TEXT("AbilityClicked");
	CurrentState->AbilityClicked(Ability);
	AttemptTransition();
}

void UTacTurnSubsystem::OnPresentationComplete()
{
	check(CurrentState);
	bAwaitingPresentation = false;
	RecordEvent(TEXT("PresentationComplete"));
	PendingTrigger = TEXT("PresentationComplete");
	CurrentState->OnPresentationComplete();
	AttemptTransition();
}

bool UTacTurnSubsystem::PresentPendingSlice()
{
	UTacLogSubsystem* Log = GetWorld()->GetSubsystem<UTacLogSubsystem>();
	const TArray<FGuid>& Spine = Log->GetSpine();
	if (Spine.IsEmpty() || Spine.Last() == LastPresentedEventId)
	{
		return false;
	}

	const FGuid To = Spine.Last();
	UPresentationSequence* Seq = PresentationBuilder->Build(LastPresentedEventId, To);
	LastPresentedEventId = To;

	if (Seq && Seq->Actions.Num() > 0)
	{
		UPresentationSequencePlayer* Player = UPresentationSequencePlayer::Get(this);
		Player->EnqueueSequence(Seq);
		Player->Play();
		return true;
	}
	return false;
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
	TArray<AUnit*> Units = GridSubsystem->GetActiveUnits();
	for (AUnit* Unit : GridSubsystem->GetOffFieldUnits())
	{
		if (!Unit->GetStats().Status.IsFleeing())
			Units.Add(Unit);
	}
	TurnOrder->Repopulate(Units, GridSubsystem->GetAttackerTeam());
	for (AUnit* Unit : Units)
	{
		Unit->OnUnitDied.RemoveDynamic(this, &UTacTurnSubsystem::HandleUnitDied);
		Unit->OnUnitDied.AddDynamic(this, &UTacTurnSubsystem::HandleUnitDied);
	}
}

void UTacTurnSubsystem::RecordPhase(ETurnState State)
{
	if (PhaseHistory.Num() < KPhaseHistorySize)
		PhaseHistory.Add(State);
	else
		PhaseHistory[PhaseHistoryIdx % KPhaseHistorySize] = State;
	++PhaseHistoryIdx;
}

void UTacTurnSubsystem::RecordTransition(ETurnState From, ETurnState To)
{
	AUnit* Unit = GetCurrentUnit();
	FTransitionRecord Rec{From, To, PendingTrigger, CurrentRound, Unit ? Unit->GetName() : TEXT("None")};
	if (TransitionHistory.Num() < KTransitionRecordSize)
		TransitionHistory.Add(Rec);
	else
		TransitionHistory[TransitionHistoryIdx % KTransitionRecordSize] = Rec;
	++TransitionHistoryIdx;
}

void UTacTurnSubsystem::RecordEvent(const FString& Event)
{
	if (EventHistory.Num() < KEventHistorySize)
		EventHistory.Add(Event);
	else
		EventHistory[EventHistoryIdx % KEventHistorySize] = Event;
	++EventHistoryIdx;
}

void UTacTurnSubsystem::DumpInfiniteLoopDiagnostics() const
{
	const UEnum* StateEnum = StaticEnum<ETurnState>();
	AUnit* Unit = GetCurrentUnit();

	UE_LOG(LogKBSTurn, Error, TEXT("=== INFINITE LOOP DETECTED (100 consecutive transitions) ==="));
	UE_LOG(LogKBSTurn, Error, TEXT("Round: %d | ActiveUnit: %s"), CurrentRound,
		Unit ? *Unit->GetName() : TEXT("None"));

	UE_LOG(LogKBSTurn, Error, TEXT("--- Phase History (oldest->newest, %d entries) ---"), PhaseHistory.Num());
	const int32 PhaseStart = PhaseHistory.Num() >= KPhaseHistorySize ? PhaseHistoryIdx % KPhaseHistorySize : 0;
	for (int32 i = 0; i < PhaseHistory.Num(); ++i)
	{
		const ETurnState& Phase = PhaseHistory[(PhaseStart + i) % PhaseHistory.Num()];
		UE_LOG(LogKBSTurn, Error, TEXT("  [%d] %s"), i, *StateEnum->GetNameStringByValue(static_cast<int64>(Phase)));
	}

	UE_LOG(LogKBSTurn, Error, TEXT("--- Last %d Transition Records ---"), TransitionHistory.Num());
	const int32 TransStart = TransitionHistory.Num() >= KTransitionRecordSize ? TransitionHistoryIdx % KTransitionRecordSize : 0;
	for (int32 i = 0; i < TransitionHistory.Num(); ++i)
	{
		const FTransitionRecord& Rec = TransitionHistory[(TransStart + i) % TransitionHistory.Num()];
		UE_LOG(LogKBSTurn, Error, TEXT("  [%d] %s -> %s | Trigger: %s | Round: %d | Unit: %s"),
			i,
			*StateEnum->GetNameStringByValue(static_cast<int64>(Rec.From)),
			*StateEnum->GetNameStringByValue(static_cast<int64>(Rec.To)),
			*Rec.Trigger, Rec.Round, *Rec.UnitName);
	}

	UE_LOG(LogKBSTurn, Error, TEXT("--- Last %d Events (oldest->newest) ---"), EventHistory.Num());
	const int32 EvStart = EventHistory.Num() >= KEventHistorySize ? EventHistoryIdx % KEventHistorySize : 0;
	for (int32 i = 0; i < EventHistory.Num(); ++i)
	{
		UE_LOG(LogKBSTurn, Error, TEXT("  [%d] %s"), i, *EventHistory[(EvStart + i) % EventHistory.Num()]);
	}

	if (Unit)
	{
		UE_LOG(LogKBSTurn, Error, TEXT("--- Active Unit Abilities ---"));
		UE_LOG(LogKBSTurn, Error, TEXT("%s"), *Unit->GetAbilityInventory()->GetAbilitiesDebugString());
	}

	UE_LOG(LogKBSTurn, Error, TEXT("Recovering by forcing -> EBattleEndState"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Red,
			FString::Printf(TEXT("[TacTurn] Inf-loop detected! Round %d, Unit: %s. Forced BattleEnd. See LogKBSTurn for details."),
				CurrentRound, Unit ? *Unit->GetName() : TEXT("None")));
	}
}

static void BroadcastWithTurnChangeLog(UWorld* World, ETurnChangeKind Kind, FGuid InstigatorId, ETeamSide Team, int32 Round, int32 TurnNum, TFunctionRef<void()> Broadcast)
{
	UTacLogSubsystem* Log = World->GetSubsystem<UTacLogSubsystem>();
	FGuid Id = Log ? Log->OpenEvent(ETacLogEventType::TurnChange, ETacLogEventOrigin::Initiated, InstigatorId, FGuid(), Round, TurnNum) : FGuid();
	Broadcast();
	if (!Log) return;
	TInstancedStruct<FTacLogPayload> Payload;
	Payload.InitializeAs<FTurnChangePayload>();
	FTurnChangePayload& P = Payload.GetMutable<FTurnChangePayload>();
	P.ChangeKind   = Kind;
	P.NewTurnOwner = InstigatorId;
	P.NewTurnTeam  = Team;
	Log->CloseEvent(Id, MoveTemp(Payload));
}


void UTacTurnSubsystem::BroadcastRoundStart()
{
	BroadcastWithTurnChangeLog(GetWorld(), ETurnChangeKind::Round, FGuid(), ETeamSide::Attacker, CurrentRound, 0,
		[this]{ OnRoundStart.Broadcast(CurrentRound); });
}

void UTacTurnSubsystem::BroadcastRoundEnd()
{
	BroadcastWithTurnChangeLog(GetWorld(), ETurnChangeKind::Round, FGuid(), ETeamSide::Attacker, CurrentRound, 0,
		[this]{ OnRoundEnd.Broadcast(CurrentRound); });
}

void UTacTurnSubsystem::BroadcastTurnStart()
{
	AUnit* Unit = TurnOrder->GetCurrentUnit();
	BroadcastWithTurnChangeLog(GetWorld(), ETurnChangeKind::Turn,
		Unit ? Unit->GetUnitID() : FGuid(), Unit ? Unit->GetTeamSide() : ETeamSide::Attacker,
		CurrentRound, ++CurrentTurnNumber,
		[this, Unit]{ OnTurnStart.Broadcast(Unit); });
}

void UTacTurnSubsystem::BroadcastTurnEnd()
{
	AUnit* Unit = TurnOrder->GetCurrentUnit();
	BroadcastWithTurnChangeLog(GetWorld(), ETurnChangeKind::Turn,
		Unit ? Unit->GetUnitID() : FGuid(), Unit ? Unit->GetTeamSide() : ETeamSide::Attacker,
		CurrentRound, CurrentTurnNumber,
		[this, Unit]{ OnTurnEnd.Broadcast(Unit); });
}

void UTacTurnSubsystem::BroadcastBattleEnd()
{
	OnBattleEnd.Broadcast(GridSubsystem->GetWinnerTeam());
}
