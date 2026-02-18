#include "GameMechanics/Units/Stats/BaseUnitStatTypes.h"

// FUnitStatPercent implementations
FUnitStatPercent::FUnitStatPercent(int32 InBase)
	: Base(FMath::Clamp(InBase, 0, 100)), Modified(Base), bIsDirty(true)
{
}

bool FUnitStatPercent::IsDirty() const
{
	return bIsDirty;
}

void FUnitStatPercent::Recalc() const
{
	Modified = FMath::RoundToInt((Base + CalcFlatModifiers()) * CalcStatMultipliers());
	Modified = FMath::Clamp(Modified, 0, 100);
	bIsDirty = false;
}

int32 FUnitStatPercent::CalcFlatModifiers() const
{
	int32 Result = 0;
	for (const FInt32StatModifier& Mod : FlatModifiers)
	{
		Result += Mod.Amount;
	}
	return Result;
}

float FUnitStatPercent::CalcStatMultipliers() const
{
	float Result = 1.0f;
	for (const FInt32StatModifier& Mod : StatMultipliers)
	{
		Result += Mod.Amount / 100.0f;
	}
	return Result;
}

void FUnitStatPercent::AddFlatModifier(const FGuid& EffectId, int32 Amount)
{
	FlatModifiers.Add(FInt32StatModifier(EffectId, Amount));
	bIsDirty = true;
}

void FUnitStatPercent::AddMultiplier(const FGuid& EffectId, int32 Amount)
{
	StatMultipliers.Add(FInt32StatModifier(EffectId, Amount));
	bIsDirty = true;
}

void FUnitStatPercent::RemoveFlatModifier(const FGuid& EffectId, int32 Amount)
{
	FlatModifiers.Remove(FInt32StatModifier(EffectId, Amount));
	bIsDirty = true;
}

void FUnitStatPercent::RemoveMultiplier(const FGuid& EffectId, int32 Amount)
{
	StatMultipliers.Remove(FInt32StatModifier(EffectId, Amount));
	bIsDirty = true;
}

int32 FUnitStatPercent::GetValue() const
{
	if (bIsDirty)
		Recalc();
	return Modified;
}

void FUnitStatPercent::SetBase(const int32 NewBase)
{
	Base = FMath::Clamp(NewBase, 0, 100);
	bIsDirty = true;
}

void FUnitStatPercent::InitFromBase(int32 InBase)
{
	Base = FMath::Clamp(InBase, 0, 100);
	Modified = Base;
	FlatModifiers.Empty();
	StatMultipliers.Empty();
	bIsDirty = true;
}

// FUnitStatPositive implementations
FUnitStatPositive::FUnitStatPositive(int32 InBase)
	: Base(FMath::Max(InBase, 0)), Modified(Base), bIsDirty(true)
{
}

bool FUnitStatPositive::IsDirty() const
{
	return bIsDirty;
}

void FUnitStatPositive::Recalc() const
{
	Modified = FMath::RoundToInt((Base + CalcFlatModifiers()) * CalcStatMultipliers());
	Modified = FMath::Max(Modified, 0);
	bIsDirty = false;
}

int32 FUnitStatPositive::CalcFlatModifiers() const
{
	int32 Result = 0;
	for (const FInt32StatModifier& Mod : FlatModifiers)
	{
		Result += Mod.Amount;
	}
	return Result;
}

float FUnitStatPositive::CalcStatMultipliers() const
{
	float Result = 1.0f;
	for (const FInt32StatModifier& Mod : StatMultipliers)
	{
		Result += Mod.Amount / 100.0f;
	}
	return Result;
}

void FUnitStatPositive::AddFlatModifier(const FGuid& EffectId, int32 Amount)
{
	FlatModifiers.Add(FInt32StatModifier(EffectId, Amount));
	bIsDirty = true;
}

void FUnitStatPositive::AddMultiplier(const FGuid& EffectId, int32 Amount)
{
	StatMultipliers.Add(FInt32StatModifier(EffectId, Amount));
	bIsDirty = true;
}

void FUnitStatPositive::RemoveFlatModifier(const FGuid& EffectId, int32 Amount)
{
	FlatModifiers.Remove(FInt32StatModifier(EffectId, Amount));
	bIsDirty = true;
}

void FUnitStatPositive::RemoveMultiplier(const FGuid& EffectId, int32 Amount)
{
	StatMultipliers.Remove(FInt32StatModifier(EffectId, Amount));
	bIsDirty = true;
}

int32 FUnitStatPositive::GetValue() const
{
	if (bIsDirty)
		Recalc();
	return Modified;
}

void FUnitStatPositive::SetBase(const int32 NewBase)
{
	Base = FMath::Max(NewBase, 0);
	bIsDirty = true;
}

void FUnitStatPositive::InitFromBase(int32 InBase)
{
	Base = FMath::Max(InBase, 0);
	Modified = Base;
	FlatModifiers.Empty();
	StatMultipliers.Empty();
	bIsDirty = false;
}

// FDamageSourceSetStat implementations
FDamageSourceSetStat::FDamageSourceSetStat(const TSet<EDamageSource>& InBase)
	: Base(InBase), Modified(InBase), bIsDirty(true)
{
}

bool FDamageSourceSetStat::IsDirty() const
{
	return bIsDirty;
}

void FDamageSourceSetStat::Recalc() const
{
	Modified = Base;
	for (const FDamageSourceSetModifier& Mod : Modifiers)
	{
		Modified.Append(Mod.Sources);
	}
	bIsDirty = false;
}

void FDamageSourceSetStat::AddModifier(const FGuid& EffectId, const TSet<EDamageSource>& Sources)
{
	Modifiers.Add(FDamageSourceSetModifier(EffectId, Sources));
	bIsDirty = true;
}

void FDamageSourceSetStat::RemoveModifier(const FGuid& EffectId)
{
	Modifiers.RemoveAll([&EffectId](const FDamageSourceSetModifier& Mod)
	{
		return Mod.EffectId == EffectId;
	});
	bIsDirty = true;
}

const TSet<EDamageSource>& FDamageSourceSetStat::GetValue() const
{
	if (bIsDirty)
		Recalc();
	return Modified;
}

void FDamageSourceSetStat::SetBase(const TSet<EDamageSource>& NewBase)
{
	Base = NewBase;
	bIsDirty = true;
}

void FDamageSourceSetStat::InitFromBase(const TSet<EDamageSource>& InBase)
{
	Base = InBase;
	Modified = InBase;
	Modifiers.Empty();
	bIsDirty = true;
}
