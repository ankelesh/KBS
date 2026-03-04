#include "GameMechanics/Units/Weapons/Weapon.h"
#include "GameMechanics/Units/Weapons/WeaponDataAsset.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "GameMechanics/Units/Unit.h"
void UWeapon::Initialize(UObject* Outer, UWeaponDataAsset* Data, int32 BaseDamageOverride)
{
	if (!Data)
	{
		UE_LOG(LogTemp, Error, TEXT("FWeapon::Initialize - No WeaponDataAsset provided"));
		return;
	}
	Config = Data;
	Designation = Data->Designation;
	Stats = Data->BaseStats;
	if (BaseDamageOverride != -1)
		Stats.BaseDamage.SetBase(BaseDamageOverride)
	bIsImmutable = Data->bIsImmutable;
	bGuaranteedHit = Data->bGuaranteedHit;
	Intent = Data->Intent;
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

EAttackIntent UWeapon::DeduceAttackIntent(const UWeapon* Weapon)
{
	if (Weapon->Intent == EAttackIntent::Auto)
	{
		if (!Weapon->IsMutable() && Weapon->Stats.BaseDamage.GetBase() == 0 && Weapon->ActiveEffects.Num() > 0)
			return EAttackIntent::EffectApplication;
		return EAttackIntent::Attack;
	}
	return Weapon->GetIntent();
}

void UWeapon::ModifyDamage(int32 Damage, FGuid ModificatorGuid, bool bIsFlat)
{
	if (!bIsImmutable)
		if (bIsFlat)
			Stats.BaseDamage.AddFlatModifier(ModificatorGuid, Damage);
		else
			Stats.BaseDamage.AddMultiplier(ModificatorGuid, Damage);
}

void UWeapon::RemoveDamageModifier(int32 Damage, FGuid ModificatorGuid, bool bIsFlat)
{
	if (!bIsImmutable)
		if (bIsFlat)
			Stats.BaseDamage.RemoveFlatModifier(ModificatorGuid, Damage);
		else
			Stats.BaseDamage.RemoveMultiplier(ModificatorGuid, Damage);
}

void UWeapon::RemoveSourceModifier(FGuid ModificatorGuid)
{
	if (!bIsImmutable)
		Stats.DamageSources.RemoveModifier(ModificatorGuid);
}

void UWeapon::RemoveDamageModifier(FGuid ModificatorGuid, bool bIsFlat)
{
}

void UWeapon::RemoveSourceModifier(FGuid ModificatorGuid)
{
}

void UWeapon::ModifySource(const TSet<EDamageSource>& Sources, FGuid ModificatorGuid)
{
	if (!bIsImmutable)
	{
		Stats.DamageSources.AddModifier(ModificatorGuid, Sources);
	}
}

bool UWeapon::IsMutable() const
{
	return bIsImmutable;
}

EAttackIntent UWeapon::GetIntent() const
{
	if (Intent == EAttackIntent::Auto)
		return DeduceAttackIntent(this);
	return Intent;
}

FText UWeapon::GetEffectsTooltips(AUnit* Owner)
{
	return FText();
}
