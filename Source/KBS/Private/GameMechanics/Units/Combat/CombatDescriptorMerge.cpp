#include "GameMechanics/Units/Combat/CombatDescriptorMerge.h"
#include "GameMechanics/Units/Combat/CombatDescriptorDataAsset.h"
#include "GameplayTypes/DescriptorOverridePolicies.h"

FCombatDescriptorData MergeDescriptors(const FWeaponData& Weapon)
{
	const EDescriptorOverridePolicy Policy = Weapon.OverridePolicy;
	FCombatDescriptorData Result;

	if (Policy == EDescriptorOverridePolicy::AssetOnly)
	{
		checkf(Weapon.DescriptorAsset != nullptr, TEXT("MergeDescriptors: AssetOnly policy requires a non-null DescriptorAsset"));
		Result = Weapon.DescriptorAsset->Data;
	}
	else if (Policy == EDescriptorOverridePolicy::InlineOnly || Weapon.DescriptorAsset == nullptr)
	{
		Result = Weapon.InlineDescriptor;
	}
	else
	{
		const FCombatDescriptorData& Asset  = Weapon.DescriptorAsset->Data;
		const FCombatDescriptorData& Inline = Weapon.InlineDescriptor;

		Result.Name            = PickText(Asset.Name, Inline.Name, Policy);
		Result.MagnitudePolicy = PickScalar(Asset.MagnitudePolicy, Inline.MagnitudePolicy, EMagnitudePolicy::None, Policy);
		Result.bIsImmutable    = PickBool(Asset.bIsImmutable, Inline.bIsImmutable, Policy);
		Result.bGuaranteedHit  = PickBool(Asset.bGuaranteedHit, Inline.bGuaranteedHit, Policy);
		Result.Effects         = PickContainer(Asset.Effects, Inline.Effects, Policy);
		Result.SideEffects     = !Inline.SideEffects.IsActive() ? Asset.SideEffects : Inline.SideEffects;

		Result.BaseStats.BaseMagnitude      = PickIntegralStat(Asset.BaseStats.BaseMagnitude, Inline.BaseStats.BaseMagnitude, -1, Policy);
		Result.BaseStats.AccuracyMultiplier = PickScalar(Asset.BaseStats.AccuracyMultiplier, Inline.BaseStats.AccuracyMultiplier, -1, Policy);
		Result.BaseStats.DamageSources      = PickSetStat(Asset.BaseStats.DamageSources, Inline.BaseStats.DamageSources, Policy);
		Result.BaseStats.TargetReach        = PickScalar(Asset.BaseStats.TargetReach, Inline.BaseStats.TargetReach, ETargetReach::None, Policy);
		Result.BaseStats.AreaShape          = (Inline.BaseStats.TargetReach == ETargetReach::None) ? Asset.BaseStats.AreaShape : Inline.BaseStats.AreaShape;
	}

	if (Weapon.DamageOverride != -1)
		Result.BaseStats.BaseMagnitude.InitFromBase(Weapon.DamageOverride);

	return Result;
}
