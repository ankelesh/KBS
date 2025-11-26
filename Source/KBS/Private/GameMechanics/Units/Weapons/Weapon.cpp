#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"

UWeapon::UWeapon()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeapon::InitializeFromDataAsset()
{
	if (!Config)
	{
		return;
	}

	BaseStats = Config->BaseStats;
	ModifiedStats = BaseStats;

	ActiveEffects.Empty();
	// Note: BattleEffects are spawned from EffectClasses but not initialized with DataAsset
	// since they don't store DataAsset references in the weapon config
	for (const TSubclassOf<UBattleEffect>& EffectClass : Config->EffectClasses)
	{
		if (EffectClass)
		{
			UBattleEffect* NewEffect = NewObject<UBattleEffect>(this, EffectClass);
			if (NewEffect)
			{
				ActiveEffects.Add(NewEffect);
			}
		}
	}
}

void UWeapon::RecalculateModifiedStats()
{
	ModifiedStats = BaseStats;
}

void UWeapon::Restore()
{
	ModifiedStats = BaseStats;
}
