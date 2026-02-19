#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/BattleEndState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"

void FBattleEndState::Enter()
{
	UE_LOG(LogKBSTurn, Log, TEXT("Battle ended"));
	BroadcastBattleEnd();
}

void FBattleEndState::Exit()
{
	UE_LOG(LogTemp, Warning, TEXT("Someone tries to tranzit out FBattleEndState"));
}

ETurnState FBattleEndState::NextState()
{
	return ETurnState::EBattleEndState;
}
