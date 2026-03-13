#include "GameMechanics/Units/Combat/CombatDescriptor.h"
#include "GameMechanics/Units/Combat/CombatDescriptorDataAsset.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "GameMechanics/Units/Unit.h"
void UCombatDescriptor::Initialize(UObject* Outer, UCombatDescriptorDataAsset* Data, int32 BaseMagnitudeOverride)
{
	if (!Data)
	{
		UE_LOG(LogTemp, Error, TEXT("UCombatDescriptor::Initialize - No DataAsset provided"));
		return;
	}
	Config = Data;
	Designation = Data->Designation;
	Stats = Data->BaseStats;
	if (BaseMagnitudeOverride != -1)
		Stats.BaseMagnitude.SetBase(BaseMagnitudeOverride);
	bIsImmutable = Data->bIsImmutable;
	bGuaranteedHit = Data->bGuaranteedHit;
	MagnitudePolicy = Data->MagnitudePolicy;
	ActiveEffects.Empty();
	for (const FDescriptorEffectConfig& EffectConfig : Data->Effects)
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


void UCombatDescriptor::ModifyMagnitude(int32 Magnitude, FGuid ModificatorGuid, bool bIsFlat)
{
	if (!bIsImmutable)
		if (bIsFlat)
			Stats.BaseMagnitude.AddFlatModifier(ModificatorGuid, Magnitude);
		else
			Stats.BaseMagnitude.AddMultiplier(ModificatorGuid, Magnitude);
}

void UCombatDescriptor::RemoveMagnitudeModifier(int32 Magnitude, FGuid ModificatorGuid, bool bIsFlat)
{
	if (!bIsImmutable)
		if (bIsFlat)
			Stats.BaseMagnitude.RemoveFlatModifier(ModificatorGuid, Magnitude);
		else
			Stats.BaseMagnitude.RemoveMultiplier(ModificatorGuid, Magnitude);
}

void UCombatDescriptor::RemoveSourceModifier(FGuid ModificatorGuid)
{
	if (!bIsImmutable)
		Stats.DamageSources.RemoveModifier(ModificatorGuid);
}



void UCombatDescriptor::ModifySource(const TSet<EDamageSource>& Sources, FGuid ModificatorGuid)
{
	if (!bIsImmutable)
	{
		Stats.DamageSources.AddModifier(ModificatorGuid, Sources);
	}
}

bool UCombatDescriptor::IsMutable() const
{
	return bIsImmutable;
}

void UCombatDescriptor::SetMagnitudeBase(int32 Magnitude)
{
	if (!bIsImmutable)
		Stats.BaseMagnitude.SetBase(Magnitude);
}


FText UCombatDescriptor::GetEffectsTooltips(AUnit* Owner)
{
	return FText();
}
