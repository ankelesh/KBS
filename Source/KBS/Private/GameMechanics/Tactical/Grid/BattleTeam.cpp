#include "GameMechanics/Tactical/Grid/BattleTeam.h"
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
