#include "GameMechanics/Units/Stats/UnitHealth.h"

void FUnitHealth::SetMaxBase(int32 NewBase, bool bShouldHeal)
{
	int32 OldMax = Maximum.GetValue();
	Maximum.SetBase(NewBase);
	int32 NewMax = Maximum.GetValue();
	int32 Delta = NewMax - OldMax;

	if (Delta > 0 && bShouldHeal)
	{
		Current += Delta;
	}
	else if (Delta < 0)
	{
		ClampCurrent();
	}
}

void FUnitHealth::AddMaxModifier(const FGuid& EffectId, int32 Amount, bool bShouldHeal)
{
	int32 OldMax = Maximum.GetValue();
	Maximum.AddFlatModifier(EffectId, Amount);
	int32 NewMax = Maximum.GetValue();
	int32 Delta = NewMax - OldMax;

	if (Delta > 0 && bShouldHeal)
	{
		Current += Delta;
	}
	else if (Delta < 0)
	{
		ClampCurrent();
	}
}

void FUnitHealth::AddMaxMultiplier(const FGuid& EffectId, int32 Amount, bool bShouldHeal)
{
	int32 OldMax = Maximum.GetValue();
	Maximum.AddMultiplier(EffectId, Amount);
	int32 NewMax = Maximum.GetValue();
	int32 Delta = NewMax - OldMax;

	if (Delta > 0 && bShouldHeal)
	{
		Current += Delta;
	}
	else if (Delta < 0)
	{
		ClampCurrent();
	}
}

void FUnitHealth::RemoveMaxModifier(const FGuid& EffectId, int32 Amount)
{
	Maximum.RemoveFlatModifier(EffectId, Amount);
	ClampCurrent();
}

void FUnitHealth::RemoveMaxMultiplier(const FGuid& EffectId, int32 Amount)
{
	Maximum.RemoveMultiplier(EffectId, Amount);
	ClampCurrent();
}



void FUnitHealth::SetCurrent(int32 NewCurrent)
{
	Current = NewCurrent;
	ClampCurrent();
}

void FUnitHealth::ApplyDelta(int32 Amount)
{
	Current += Amount;
	ClampCurrent();
}

void FUnitHealth::FullHeal()
{
	Current = Maximum.GetValue();
}

int32 FUnitHealth::GetMaximum() const
{
	return Maximum.GetValue();
}

float FUnitHealth::GetHealthPercent() const
{
	int32 Max = Maximum.GetValue();
	if (Max <= 0)
		return 0.0f;
	return static_cast<float>(Current) / static_cast<float>(Max);
}

void FUnitHealth::ClampCurrent()
{
	int32 Max = Maximum.GetValue();
	Current = FMath::Clamp(Current, 0, Max);
}

void FUnitHealth::InitFromBase(const FUnitHealth& Template)
{
	Maximum.InitFromBase(Template.Maximum.GetBase());
	Current = Maximum.GetValue();
}
