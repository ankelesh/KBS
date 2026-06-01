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
	if (Delta.MaxHealth != 0)
		Health.AddMaxModifier(EffectId, Delta.MaxHealth, true);
	if (Delta.Initiative != 0)
		Initiative.AddFlatModifier(EffectId, Delta.Initiative);
	if (Delta.Accuracy != 0)
		Accuracy.AddFlatModifier(EffectId, Delta.Accuracy);

	for (EDamageSource Immunity : Delta.ImmunitiesToGrant)
		Defense.Immunities.AddModifier(EffectId, Immunity, true);

	for (const auto& [Source, Amount] : Delta.ArmourDeltas)
		if (Amount != 0)
			Defense.Armour.AddFlatModifier(EffectId, Amount, Source);
}

void FUnitCoreStats::RemoveDelta(const FUnitStatDelta& Delta, const FGuid& EffectId)
{
	if (Delta.MaxHealth != 0)
		Health.RemoveMaxModifier(EffectId, Delta.MaxHealth);
	if (Delta.Initiative != 0)
		Initiative.RemoveFlatModifier(EffectId, Delta.Initiative);
	if (Delta.Accuracy != 0)
		Accuracy.RemoveFlatModifier(EffectId, Delta.Accuracy);

	for (EDamageSource Immunity : Delta.ImmunitiesToGrant)
		Defense.Immunities.RemoveModifier(EffectId, Immunity, true);

	for (const auto& [Source, Amount] : Delta.ArmourDeltas)
		if (Amount != 0)
			Defense.Armour.RemoveFlatModifier(EffectId, Amount, Source);
}
