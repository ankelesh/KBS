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
		if (!Unit->GetStats().Health.IsDead())
			return true;
	}
	return false;
}
