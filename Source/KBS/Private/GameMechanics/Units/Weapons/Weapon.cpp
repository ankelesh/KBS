#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/UnitVisualsComponent.h"
#include "Engine/StaticMesh.h"

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
	BaseStats = Data->BaseStats;
	ModifiedStats = BaseStats;

	ActiveEffects.Empty();
	for (const TSubclassOf<UBattleEffect>& EffectClass : Data->EffectClasses)
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

	if (VisualsComp && !Data->WeaponMesh.IsNull())
	{
		UStaticMesh* LoadedMesh = Data->WeaponMesh.LoadSynchronous();
		if (LoadedMesh)
		{
			VisualsComp->AttachWeaponMesh(LoadedMesh, Data->AttachSocketName);
		}
	}
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
