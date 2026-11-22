#include "GameMechanics/Units/Abilities/Defaults/ChargedAutoAttackAbility.h"
#include "GameMechanics/Units/Abilities/Defaults/ChargedAutoAttackAbilityDefinition.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"
#include "GameMechanics/Units/Components/UnitChargeComponent.h"
#include "GameMechanics/Units/Unit.h"

void UChargedAutoAttackAbility::InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner)
{
	Super::InitializeFromDefinition(InDefinition, InOwner);

	const UChargedAutoAttackAbilityDefinition& ChargedConfig = GetChargedConfig();
	checkf(Owner->GetCharges().IsTracked(ChargedConfig.ChargeCost.Tag),
	       TEXT("%s: charge '%s' is not tracked by %s"), *Config->GetName(), *ChargedConfig.ChargeCost.Tag.ToString(),
	       *Owner->GetLogName());

	ChargedDescriptor = NewObject<UCombatDescriptor>(this);
	ChargedDescriptor->Initialize(this, ChargedConfig.ChargedDescriptor);
}

UCombatDescriptor* UChargedAutoAttackAbility::ResolveAttackDescriptor(UCombatDescriptor* WeaponDescriptor) const
{
	return CanAffordCharge() ? ChargedDescriptor.Get() : WeaponDescriptor;
}

void UChargedAutoAttackAbility::OnAttackResolved(const UCombatDescriptor* UsedDescriptor,
                                                 TArray<TInstancedStruct<FTacLogStepBase>>& OutSteps)
{
	if (UsedDescriptor == ChargedDescriptor)
	{
		const FUnitChargeCost& Cost = GetChargedConfig().ChargeCost;
		Owner->GetCharges().DecrementCharge(Cost.Tag, Cost.Amount);
		AppendChargeStep(OutSteps, Cost.Tag, -Cost.Amount);
	}
}

void UChargedAutoAttackAbility::GatherTagDescriptors(UCombatDescriptor* WeaponDescriptor,
                                                     TArray<UCombatDescriptor*>& OutDescriptors) const
{
	Super::GatherTagDescriptors(WeaponDescriptor, OutDescriptors);
	OutDescriptors.Add(ChargedDescriptor);
}

const UChargedAutoAttackAbilityDefinition& UChargedAutoAttackAbility::GetChargedConfig() const
{
	const UChargedAutoAttackAbilityDefinition* ChargedConfig = Cast<UChargedAutoAttackAbilityDefinition>(Config);
	checkf(ChargedConfig && ChargedConfig->ChargeCost.Tag.IsValid(),
	       TEXT("UChargedAutoAttackAbility requires a UChargedAutoAttackAbilityDefinition with ChargeCost.Tag"));
	return *ChargedConfig;
}

bool UChargedAutoAttackAbility::CanAffordCharge() const
{
	const FUnitChargeCost& Cost = GetChargedConfig().ChargeCost;
	return Owner->GetCharges().HasCharge(Cost.Tag, Cost.Amount);
}
