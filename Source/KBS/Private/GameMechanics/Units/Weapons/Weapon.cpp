#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "GameMechanics/Units/Unit.h"
UWeapon::UWeapon()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UWeapon::Initialize(UUnitVisualsComponent* VisualsComp, UWeaponDataAsset* Data)
{
	if (!Data)
	{
		UE_LOG(LogTemp, Error, TEXT("UWeapon::Initialize - No WeaponDataAsset provided"));
		return;
	}
	Config = Data;
	Stats = Data->BaseStats;
	ActiveEffects.Empty();
	for (const FWeaponEffectConfig& EffectConfig : Data->Effects)
	{
		if (EffectConfig.EffectClass && EffectConfig.EffectConfig)
		{
			UBattleEffect* NewEffect = NewObject<UBattleEffect>(this, EffectConfig.EffectClass);
			if (NewEffect)
			{
				NewEffect->Initialize(EffectConfig.EffectConfig);
				ActiveEffects.Add(NewEffect);
			}
		}
	}
}
void UWeapon::InitializeFromDataAsset()
{
	if (!Config)
	{
		return;
	}
	Stats = Config->BaseStats;
	ActiveEffects.Empty();
	for (const FWeaponEffectConfig& EffectConfig : Config->Effects)
	{
		if (EffectConfig.EffectClass && EffectConfig.EffectConfig)
		{
			UBattleEffect* NewEffect = NewObject<UBattleEffect>(this, EffectConfig.EffectClass);
			if (NewEffect)
			{
				NewEffect->Initialize(EffectConfig.EffectConfig);
				ActiveEffects.Add(NewEffect);
			}
		}
	}
}

FText UWeapon::GetEffectsTooltips(AUnit* Owner)
{
}
