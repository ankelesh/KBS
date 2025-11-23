#include "GameMechanics/Units/Weapons/Weapon.h"

UWeapon::UWeapon()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeapon::RecalculateModifiedStats()
{
	ModifiedStats = BaseStats;
}

void UWeapon::Restore()
{
	ModifiedStats = BaseStats;
}
