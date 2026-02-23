#include "GameMechanics/Units/Abilities/Passives/SummonedPassiveAbility.h"
#include "GameMechanics/Units/Unit.h"

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
		DespawnSelf();
	}
}

void USummonedPassiveAbility::HandleSummonerDied(AUnit* Unit)
{
	DespawnSelf();
}

void USummonedPassiveAbility::HandleOwnerDied(AUnit* Unit)
{
	Unsubscribe();
}

void USummonedPassiveAbility::DespawnSelf()
{
	if (Owner->IsDead()) return;
	Unsubscribe();
	Owner->HandleDeath();
}
