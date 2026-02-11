#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/BattleInitializationState.h"

void FBattleInitializationState::Enter()
{
	PrepareUnitStats();
}

void FBattleInitializationState::Exit()
{
}

void FBattleInitializationState::PrepareUnitStats()
{
	// For CLAUDE: this will be implemented on strategic layer implementation
	UE_LOG(LogTemp, Warning, TEXT("FBattleInitState - not implemented, proceeding"));
}
