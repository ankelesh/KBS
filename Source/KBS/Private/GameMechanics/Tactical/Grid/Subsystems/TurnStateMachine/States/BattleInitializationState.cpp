#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/BattleInitializationState.h"



inline void FBattleInitializationState::Enter()
{
	PrepareUnitStats();
}

inline void FBattleInitializationState::Exit()
{
}

inline ETurnProcessingSubstate FBattleInitializationState::CanReleaseState()
{
	return FTacTurnState::CanReleaseState();
}

inline FBattleInitializationState::FBattleInitializationState()
{
}

void FBattleInitializationState::PrepareUnitStats()
{
	// For CLAUDE: this will be implemented on strategic layer implementation
	UE_LOG(LogTemp, Error, TEXT("FBattleInitState - not implemented, proceeding"));
}
