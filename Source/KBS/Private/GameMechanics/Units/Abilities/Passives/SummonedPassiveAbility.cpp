#include "GameMechanics/Units/Abilities/Passives/SummonedPassiveAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"

void USummonedPassiveAbility::InitAsSummonedPassive(AUnit* InOwner, AUnit* InSummoner,
                                                     bool bInDespawnOnSummonerDeath, int32 InDurationTurns)
{
	Owner = InOwner;
	Summoner = InSummoner;
	bDespawnOnSummonerDeath = bInDespawnOnSummonerDeath;
	RemainingTurns = InDurationTurns;
}

void USummonedPassiveAbility::Subscribe()
{
	Owner->OnUnitDied.AddDynamic(this, &USummonedPassiveAbility::HandleOwnerDied);
	if (bDespawnOnSummonerDeath && Summoner.IsValid())
	{
		Summoner->OnUnitDied.AddDynamic(this, &USummonedPassiveAbility::HandleSummonerDied);
	}
}

void USummonedPassiveAbility::Unsubscribe()
{
	Owner->OnUnitDied.RemoveDynamic(this, &USummonedPassiveAbility::HandleOwnerDied);
	if (Summoner.IsValid())
	{
		Summoner->OnUnitDied.RemoveDynamic(this, &USummonedPassiveAbility::HandleSummonerDied);
	}
}

void USummonedPassiveAbility::HandleTurnEnd()
{
	if (RemainingTurns > 0 && --RemainingTurns == 0)
	{
		DespawnSelf(EUnitDespawnReason::DurationExpired);
	}
}

void USummonedPassiveAbility::HandleSummonerDied(AUnit* Unit)
{
	DespawnSelf(EUnitDespawnReason::SummonerDied);
}

void USummonedPassiveAbility::HandleOwnerDied(AUnit* Unit)
{
	Unsubscribe();
}

void USummonedPassiveAbility::DespawnSelf(EUnitDespawnReason Reason)
{
	if (Owner->IsDead()) return;
	Unsubscribe();

	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	check(LogSubsystem);
	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::UnitDespawn, ETacLogEventOrigin::Triggered,
	                                        Owner->GetUnitID(), FGuid());

	GetGridSubsystem()->DespawnUnit(Owner);

	FUnitDespawnPayload Payload;
	Payload.UnitId   = Owner->GetUnitID();
	Payload.TeamSide = Owner->GetTeamSide();
	Payload.Reason   = Reason;
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FUnitDespawnPayload>(MoveTemp(Payload)));
}
