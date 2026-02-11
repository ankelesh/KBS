#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/BattleEndState.h"

void FBattleEndState::Enter()
{
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
