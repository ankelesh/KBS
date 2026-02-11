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

void FUnitImmunities::Recalc() const
{
	if (bIsDirty)
	{
		ModifiedImmunities = BaseImmunities.Union(CalcAdditions()).Difference(CalcSubtractions());
		bIsDirty = false;
	}
}

TSet<EDamageSource> FUnitImmunities::CalcAdditions() const
{
	TSet<EDamageSource> AdditionsMap;
	for (const FUnitImmunityModifier& Mod : ModifyingEffects)
	{
		if (Mod.bIsGranting)
			AdditionsMap.Add(Mod.DamageSource);
	}
	return AdditionsMap;
}

TSet<EDamageSource> FUnitImmunities::CalcSubtractions() const
{
	TSet<EDamageSource> SubtractionsMap;
	for (const FUnitImmunityModifier& Mod : ModifyingEffects)
	{
		if (!Mod.bIsGranting)
			SubtractionsMap.Add(Mod.DamageSource);
	}
	return SubtractionsMap;
}

void FUnitImmunities::AddModifier(const FGuid& Id, EDamageSource Source, bool bIsGranting)
{
	ModifyingEffects.Add(FUnitImmunityModifier(Id, Source, bIsGranting));
	bIsDirty = true;
}

void FUnitImmunities::RemoveModifier(const FGuid& Id, EDamageSource Source, bool bIsGranting)
{
	ModifyingEffects.Remove(FUnitImmunityModifier(Id, Source, bIsGranting));
	bIsDirty = true;
}

bool FUnitImmunities::IsImmuneTo(EDamageSource Source) const
{
	if (bIsDirty)
		Recalc();
	return ModifiedImmunities.Contains(Source);
}

void FUnitImmunities::InitFromBase(const FUnitImmunities& Template)
{
	BaseImmunities = Template.BaseImmunities;
	ModifiedImmunities = BaseImmunities;
	ModifyingEffects.Empty();
	bIsDirty = false;
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

void FUnitWards::InitFromBase(const FUnitWards& Template)
{
	Wards = Template.Wards;
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

void FUnitArmour::Recalc() const
{
	if (!bIsDirty) return;
	ModifiedArmour = BaseArmour;
	CalcFlatModifiers();
	CalcArmourMultipliers();
	CalcOverrides();
	ClampArmour();
	bIsDirty = false;
}

void FUnitArmour::CalcFlatModifiers() const
{
	for (const FUnitArmourModifier& Mod : FlatModifiers)
	{
		ModifiedArmour[Mod.Source] += Mod.Amount;
	}
}

void FUnitArmour::CalcArmourMultipliers() const
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

void FUnitArmour::CalcOverrides() const
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

void FUnitArmour::ClampArmour() const
{
	for (auto& Pair : ModifiedArmour)
	{
		if (Pair.Value > ARMOUR_MAXIMUM)
			Pair.Value = ARMOUR_MAXIMUM;
	}
}

void FUnitArmour::AddFlatModifier(const FGuid& EffectId, int32 Amount, EDamageSource Source)
{
	FlatModifiers.Add(FUnitArmourModifier(Amount, EffectId, Source));
	bIsDirty = true;
}

void FUnitArmour::AddMultiplier(const FGuid& EffectId, int32 Amount, EDamageSource Source)
{
	MultiplierModifiers.Add(FUnitArmourModifier(Amount, EffectId, Source));
	bIsDirty = true;
}

void FUnitArmour::AddOverride(const FGuid& EffectId, int32 Amount, EDamageSource Source)
{
	OverridingModifiers.Add(FUnitArmourModifier(Amount, EffectId, Source));
	bIsDirty = true;
}

void FUnitArmour::RemoveFlatModifier(const FGuid& EffectId, int32 Amount, EDamageSource Source)
{
	FlatModifiers.Remove(FUnitArmourModifier(Amount, EffectId, Source));
	bIsDirty = true;
}

void FUnitArmour::RemoveMultiplier(const FGuid& EffectId, int32 Amount, EDamageSource Source)
{
	MultiplierModifiers.Remove(FUnitArmourModifier(Amount, EffectId, Source));
	bIsDirty = true;
}

void FUnitArmour::RemoveOverride(const FGuid& EffectId, int32 Amount, EDamageSource Source)
{
	OverridingModifiers.Remove(FUnitArmourModifier(Amount, EffectId, Source));
	bIsDirty = true;
}

int32 FUnitArmour::GetValue(EDamageSource Src) const
{
	if (bIsDirty)
		Recalc();
	return ModifiedArmour[Src];
}

void FUnitArmour::SetBase(int32 NewBase, EDamageSource Src)
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

void FUnitArmour::InitFromBase(const FUnitArmour& Template)
{
	BaseArmour = Template.BaseArmour;
	ModifiedArmour = BaseArmour;
	FlatModifiers.Empty();
	MultiplierModifiers.Empty();
	OverridingModifiers.Empty();
	bIsDirty = false;
}

// FUnitDefenseStats implementations
void FUnitDefenseStats::InitFromBase(const FUnitDefenseStats& Template)
{
	Wards.InitFromBase(Template.Wards);
	Immunities.InitFromBase(Template.Immunities);
	Armour.InitFromBase(Template.Armour);
	// DamageReduction is immutable but we can copy the value directly
	const_cast<int32&>(DamageReduction.Value) = Template.DamageReduction.Value;
	bIsDefending = false;
}
