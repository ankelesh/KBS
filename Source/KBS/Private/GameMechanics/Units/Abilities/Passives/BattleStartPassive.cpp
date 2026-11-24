#include "GameMechanics/Units/Abilities/Passives/BattleStartPassive.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"

void UBattleStartPassive::Subscribe()
{
	UTacTurnSubsystem* TurnSubsystem = GetTurnSubsystem();
	checkf(TurnSubsystem, TEXT("UBattleStartPassive: TacTurnSubsystem unavailable"));
	if (TurnSubsystem->HasBattleStarted())
		HandleBattleStart();
	else
		TurnSubsystem->OnBattleStart.AddUniqueDynamic(this, &UBattleStartPassive::HandleBattleStart);
}

void UBattleStartPassive::Unsubscribe()
{
	UTacTurnSubsystem* TurnSubsystem = GetTurnSubsystem();
	checkf(TurnSubsystem, TEXT("UBattleStartPassive: TacTurnSubsystem unavailable"));
	TurnSubsystem->OnBattleStart.RemoveDynamic(this, &UBattleStartPassive::HandleBattleStart);
}

void UBattleStartPassive::HandleBattleStart()
{
	if (bBattleStartHandled)
		return;
	bBattleStartHandled = true;
	OnBattleStart();
}
