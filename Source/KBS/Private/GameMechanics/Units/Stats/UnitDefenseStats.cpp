#include "GameMechanics/Units/Stats/UnitDefenseStats.h"

// FUnitImmunities implementations
FUnitImmunities::FUnitImmunities()
	: BaseImmunities(), ModifiedImmunities(), ModifyingEffects(), bIsDirty(false)
{
}

FUnitImmunities::FUnitImmunities(TSet<EDamageSource> Immunities)
	: BaseImmunities(Immunities), ModifiedImmunities(Immunities), ModifyingEffects(), bIsDirty(false)
{
}

void FUnitImmunities::Recalc()
{
	if (bIsDirty)
	{
		ModifiedImmunities = BaseImmunities.Union(CalcAdditions()).Difference(CalcSubtractions());
		bIsDirty = false;
	}
}

TSet<EDamageSource> FUnitImmunities::CalcAdditions()
{
	TSet<EDamageSource> AdditionsMap;
	for (const FUnitImmunityModifier& Mod : ModifyingEffects)
	{
		if (Mod.bIsGranting)
			AdditionsMap.Add(Mod.DamageSource);
	}
	return AdditionsMap;
}

TSet<EDamageSource> FUnitImmunities::CalcSubtractions()
{
	TSet<EDamageSource> SubtractionsMap;
	for (const FUnitImmunityModifier& Mod : ModifyingEffects)
	{
		if (!Mod.bIsGranting)
			SubtractionsMap.Add(Mod.DamageSource);
	}
	return SubtractionsMap;
}

void FUnitImmunities::AddModifier(const FGuid Id, const EDamageSource Source, bool bIsGranting)
{
	ModifyingEffects.Add(FUnitImmunityModifier(Id, Source, bIsGranting));
	bIsDirty = true;
}

void FUnitImmunities::RemoveModifier(const FGuid Id, const EDamageSource Source, bool bIsGranting)
{
	ModifyingEffects.Remove(FUnitImmunityModifier(Id, Source, bIsGranting));
	bIsDirty = true;
}

bool FUnitImmunities::IsImmuneTo(EDamageSource Source)
{
	if (bIsDirty)
		Recalc();
	return ModifiedImmunities.Contains(Source);
}

// FUnitWards implementations
FUnitWards::FUnitWards(TSet<EDamageSource> Source)
	: Wards(Source)
{
}

void FUnitWards::Add(EDamageSource Source)
{
	Wards.Add(Source);
}

void FUnitWards::Remove(EDamageSource Source)
{
	Wards.Remove(Source);
}

bool FUnitWards::HasWardFor(EDamageSource Source) const
{
	return Wards.Contains(Source);
}

bool FUnitWards::UseWard(EDamageSource Source)
{
	if (HasWardFor(Source))
	{
		Remove(Source);
		return true;
	}
	else
	{
		return false;
	}
}

// FUnitArmour implementations
FUnitArmour::FUnitArmour()
	: BaseArmour(InitArmourMap()), ModifiedArmour(InitArmourMap()), FlatModifiers(), MultiplierModifiers(), OverridingModifiers(), bIsDirty(false)
{
}

FUnitArmour::FUnitArmour(TMap<EDamageSource, int32> Base)
	: BaseArmour(Base), ModifiedArmour(Base), FlatModifiers(), MultiplierModifiers(), OverridingModifiers(), bIsDirty(false)
{
}

bool FUnitArmour::IsDirty() const
{
	return bIsDirty;
}

void FUnitArmour::Recalc()
{
	if (!bIsDirty) return;
	ModifiedArmour = BaseArmour;
	CalcFlatModifiers();
	CalcArmourMultipliers();
	CalcOverrides();
	ClampArmour();
	bIsDirty = false;
}

void FUnitArmour::CalcFlatModifiers()
{
	for (const FUnitArmourModifier& Mod : FlatModifiers)
	{
		ModifiedArmour[Mod.Source] += Mod.Amount;
	}
}

void FUnitArmour::CalcArmourMultipliers()
{
	TMap<EDamageSource, int32> FinalMultipliers;
	for (const FUnitArmourModifier& Mod : MultiplierModifiers)
	{
		FinalMultipliers[Mod.Source] += Mod.Amount;
	}
	TArray<EDamageSource> KeysOfMultipliers;
	FinalMultipliers.GetKeys(KeysOfMultipliers);
	for (const EDamageSource Key : KeysOfMultipliers)
	{
		ModifiedArmour[Key] = FMath::RoundToInt(ModifiedArmour[Key] * (1.0f + FinalMultipliers[Key] / 100.0f));
	}
}

void FUnitArmour::CalcOverrides()
{
	TMap<EDamageSource, int32> FinalOverrides;
	for (const FUnitArmourModifier& Mod : OverridingModifiers)
	{
		if (FinalOverrides.Contains(Mod.Source))
		{
			if (FinalOverrides[Mod.Source] < Mod.Amount)
			{
				FinalOverrides[Mod.Source] = Mod.Amount;
			}
		}
		else
		{
			FinalOverrides[Mod.Source] = Mod.Amount;
		}
	}
	for (const auto& Pair : FinalOverrides)
	{
		ModifiedArmour[Pair.Key] = Pair.Value;
	}
}

void FUnitArmour::ClampArmour()
{
	for (auto& Pair : ModifiedArmour)
	{
		if (Pair.Value > ARMOUR_MAXIMUM)
			Pair.Value = ARMOUR_MAXIMUM;
	}
}

void FUnitArmour::AddFlatModifier(const FGuid EffectId, const int32 Amount, const EDamageSource Source)
{
	FlatModifiers.Add(FUnitArmourModifier(Amount, EffectId, Source));
	bIsDirty = true;
}

void FUnitArmour::AddMultiplier(const FGuid EffectId, const int32 Amount, const EDamageSource Source)
{
	MultiplierModifiers.Add(FUnitArmourModifier(Amount, EffectId, Source));
	bIsDirty = true;
}

void FUnitArmour::AddOverride(const FGuid EffectId , const int32 Amount, const EDamageSource Source)
{
	OverridingModifiers.Add(FUnitArmourModifier(Amount, EffectId, Source));
	bIsDirty = true;
}

void FUnitArmour::RemoveFlatModifier(const FGuid EffectId, const int32 Amount, const EDamageSource Source)
{
	FlatModifiers.Remove(FUnitArmourModifier(Amount, EffectId, Source));
	bIsDirty = true;
}

void FUnitArmour::RemoveMultiplier(const FGuid EffectId, const int32 Amount, const EDamageSource Source)
{
	MultiplierModifiers.Remove(FUnitArmourModifier(Amount, EffectId, Source));
	bIsDirty = true;
}

void FUnitArmour::RemoveOverride(const FGuid EffectId , const int32 Amount, const EDamageSource Source)
{
	OverridingModifiers.Remove(FUnitArmourModifier(Amount, EffectId, Source));
	bIsDirty = true;
}

int32 FUnitArmour::GetValue(EDamageSource Src)
{
	if (bIsDirty)
		Recalc();
	return ModifiedArmour[Src];
}

void FUnitArmour::SetBase(const int32 NewBase, EDamageSource Src)
{
	BaseArmour[Src] = NewBase;
	bIsDirty = true;
}

TMap<EDamageSource, int32> FUnitArmour::InitArmourMap()
{
	TMap<EDamageSource, int32> Map;
	Map.Add(EDamageSource::Air, 0);
	Map.Add(EDamageSource::Earth, 0);
	Map.Add(EDamageSource::Water, 0);
	Map.Add(EDamageSource::Fire, 0);
	Map.Add(EDamageSource::Physical, 0);
	Map.Add(EDamageSource::Life, 0);
	Map.Add(EDamageSource::Death, 0);
	Map.Add(EDamageSource::Mind, 0);
	return Map;
}
