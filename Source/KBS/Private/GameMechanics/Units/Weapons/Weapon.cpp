#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "GameMechanics/Units/Unit.h"
void FWeapon::Initialize(UObject* Outer, UWeaponDataAsset* Data)
{
	if (!Data)
	{
		UE_LOG(LogTemp, Error, TEXT("FWeapon::Initialize - No WeaponDataAsset provided"));
		return;
	}
	Config = Data;
	Designation = Data->Designation;
	Stats = Data->BaseStats;
	ActiveEffects.Empty();
	for (const FWeaponEffectConfig& EffectConfig : Data->Effects)
	{
		if (EffectConfig.EffectClass && EffectConfig.EffectConfig)
		{
			UBattleEffect* NewEffect = NewObject<UBattleEffect>(Outer, EffectConfig.EffectClass);
			if (NewEffect)
			{
				NewEffect->Initialize(EffectConfig.EffectConfig);
				ActiveEffects.Add(NewEffect);
			}
		}
	}
}

FText FWeapon::GetEffectsTooltips(AUnit* Owner)
{
	return FText();
}
