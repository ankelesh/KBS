#include "GameMechanics/Tactical/Grid/BattleTeam.h"

#include "GameMechanics/Units/Unit.h"

void UBattleTeam::AddUnit(AUnit* Unit)
{
	if (Unit)
	{
		Units.Add(Unit);
	}
}
void UBattleTeam::RemoveUnit(AUnit* Unit)
{
	if (Unit)
	{
		Units.Remove(Unit);
	}
}
bool UBattleTeam::ContainsUnit(AUnit* Unit) const
{
	return Units.Contains(Unit);
}
void UBattleTeam::ClearUnits()
{
	Units.Empty();
}

bool UBattleTeam::IsAnyUnitAlive() const
{
	for (auto Unit : Units)
	{
		if (!Unit)
			continue;
		if (!Unit->IsDead())
			return true;
	}
	return false;
}

ETeamSide UBattleTeam::ReverseTeamSide(ETeamSide Side)
{
	switch (Side)
	{
	case ETeamSide::Attacker:
		return ETeamSide::Defender;
	case ETeamSide::Defender:
		return ETeamSide::Attacker;
	default:
		UE_LOG(LogTemp, Error, TEXT("UBattleTeam::ReverseTeamSide Unknown TeamSide when reversing"));
		return ETeamSide::Defender;
	}
}
