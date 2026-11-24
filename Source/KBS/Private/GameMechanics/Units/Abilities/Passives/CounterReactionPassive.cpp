#include "GameMechanics/Units/Abilities/Passives/CounterReactionPassive.h"
#include "GameMechanics/Units/Abilities/Passives/CounterReactionPassiveDefinition.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"

UCounterReactionPassiveDefinition::UCounterReactionPassiveDefinition()
{
	AbilityClass = UCounterReactionPassive::StaticClass();
	bIsPassive = true;
	Targeting = ETargetReach::Self;
}

void UCounterReactionPassive::InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner)
{
	Super::InitializeFromDefinition(InDefinition, InOwner);

	const UCounterReactionPassiveDefinition* CounterConfig = Cast<UCounterReactionPassiveDefinition>(Config);
	checkf(CounterConfig, TEXT("UCounterReactionPassive requires a UCounterReactionPassiveDefinition"));
	checkf(CounterConfig->bUseWeapon || CounterConfig->bUseOwnDescriptor,
	       TEXT("UCounterReactionPassive %s: needs a weapon or an own descriptor to counter with"), *CounterConfig->AbilityName);

	if (CounterConfig->bUseOwnDescriptor)
	{
		OwnDescriptor = NewObject<UCombatDescriptor>(this);
		OwnDescriptor->Initialize(this, CounterConfig->OwnDescriptor);
	}
}

void UCounterReactionPassive::Subscribe()
{
	Owner->OnUnitDamaged.AddDynamic(this, &UCounterReactionPassive::HandleOwnerDamaged);
}

void UCounterReactionPassive::Unsubscribe()
{
	Owner->OnUnitDamaged.RemoveDynamic(this, &UCounterReactionPassive::HandleOwnerDamaged);
}

void UCounterReactionPassive::HandleOwnerDamaged(AUnit* Unit, AUnit* Attacker)
{
	const bool bHasUses = Config->MaxUses == 0 || RemainingUses > 0;
	if (!bHasUses || Owner->IsDead() || Attacker->IsDead() || GetCombatSubsystem()->IsResolvingReaction())
		return;

	const UCounterReactionPassiveDefinition* CounterConfig = CastChecked<UCounterReactionPassiveDefinition>(Config);
	if (TryFireReactionAttack(Attacker, CounterConfig->bUseWeapon, OwnDescriptor, CounterConfig->OwnAnimTag))
		ConsumeUse();
}
