#include "GameMechanics/Units/Stats/UnitStats.h"

void FUnitCoreStats::InitFromBase(const FUnitCoreStats& Template)
{
	Health.InitFromBase(Template.Health);
	Initiative.InitFromBase(Template.Initiative.GetBase());
	Accuracy.InitFromBase(Template.Accuracy.GetBase());
	Defense.InitFromBase(Template.Defense);
	// Status starts clean (no modifiers)
	Status.ClearAll();
}

void FUnitCoreStats::ApplyDelta(const FUnitStatDelta& Delta, const FGuid& EffectId)
{
	switch (Delta.ApplyPolicy)
	{
	case EUnitStatDeltaApplyPolicy::FlatAdd:
		if (Delta.MaxHealth != 0)
			Health.AddMaxModifier(EffectId, Delta.MaxHealth, true);
		if (Delta.Initiative != 0)
			Initiative.AddFlatModifier(EffectId, Delta.Initiative);
		if (Delta.Accuracy != 0)
			Accuracy.AddFlatModifier(EffectId, Delta.Accuracy);
		for (const auto& [Source, Amount] : Delta.ArmourDeltas)
			if (Amount != 0)
				Defense.Armour.AddFlatModifier(EffectId, Amount, Source);
		break;

	case EUnitStatDeltaApplyPolicy::Multiplier:
		if (Delta.MaxHealth != 0)
			Health.AddMaxMultiplier(EffectId, Delta.MaxHealth, true);
		if (Delta.Initiative != 0)
			Initiative.AddMultiplier(EffectId, Delta.Initiative);
		if (Delta.Accuracy != 0)
			Accuracy.AddMultiplier(EffectId, Delta.Accuracy);
		for (const auto& [Source, Amount] : Delta.ArmourDeltas)
			if (Amount != 0)
				Defense.Armour.AddMultiplier(EffectId, Amount, Source);
		break;

	case EUnitStatDeltaApplyPolicy::Override:
		for (const auto& [Source, Amount] : Delta.ArmourDeltas)
			if (Amount != 0)
				Defense.Armour.AddOverride(EffectId, Amount, Source);
		break;
	}

	for (EDamageSource Immunity : Delta.ImmunitiesToGrant)
		Defense.Immunities.AddModifier(EffectId, Immunity, true);
}

void FUnitCoreStats::RemoveDelta(const FUnitStatDelta& Delta, const FGuid& EffectId)
{
	switch (Delta.ApplyPolicy)
	{
	case EUnitStatDeltaApplyPolicy::FlatAdd:
		if (Delta.MaxHealth != 0)
			Health.RemoveMaxModifier(EffectId, Delta.MaxHealth);
		if (Delta.Initiative != 0)
			Initiative.RemoveFlatModifier(EffectId, Delta.Initiative);
		if (Delta.Accuracy != 0)
			Accuracy.RemoveFlatModifier(EffectId, Delta.Accuracy);
		for (const auto& [Source, Amount] : Delta.ArmourDeltas)
			if (Amount != 0)
				Defense.Armour.RemoveFlatModifier(EffectId, Amount, Source);
		break;

	case EUnitStatDeltaApplyPolicy::Multiplier:
		if (Delta.MaxHealth != 0)
			Health.RemoveMaxMultiplier(EffectId, Delta.MaxHealth);
		if (Delta.Initiative != 0)
			Initiative.RemoveMultiplier(EffectId, Delta.Initiative);
		if (Delta.Accuracy != 0)
			Accuracy.RemoveMultiplier(EffectId, Delta.Accuracy);
		for (const auto& [Source, Amount] : Delta.ArmourDeltas)
			if (Amount != 0)
				Defense.Armour.RemoveMultiplier(EffectId, Amount, Source);
		break;

	case EUnitStatDeltaApplyPolicy::Override:
		for (const auto& [Source, Amount] : Delta.ArmourDeltas)
			if (Amount != 0)
				Defense.Armour.RemoveOverride(EffectId, Amount, Source);
		break;
	}

	for (EDamageSource Immunity : Delta.ImmunitiesToGrant)
		Defense.Immunities.RemoveModifier(EffectId, Immunity, true);
}
