#include "KbsGameInstance.h"

void UKbsGameInstance::SetPendingBattle(const FTacticalBattleSetup& Setup)
{
	PendingBattle = Setup;
}

void UKbsGameInstance::ClearPendingBattle()
{
	PendingBattle = FTacticalBattleSetup{};
}
