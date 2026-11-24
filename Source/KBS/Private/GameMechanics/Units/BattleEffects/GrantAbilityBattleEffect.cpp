#include "GameMechanics/Units/BattleEffects/GrantAbilityBattleEffect.h"
#include "GameMechanics/Units/Abilities/AbilityFactory.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"

void UGrantAbilityBattleEffect::Initialize(UBattleEffectDataAsset* InConfig)
{
	Super::Initialize(InConfig);
	Duration = Config->DefaultDuration;
}

void UGrantAbilityBattleEffect::OnApplied()
{
	UUnitAbilityDefinition* Definition = CastChecked<UGrantAbilityBattleEffectDataAsset>(Config)->GrantedAbility;
	checkf(Definition, TEXT("GrantAbilityBattleEffect %s: GrantedAbility not set"), *Config->GetName());
	GrantedAbility = UAbilityFactory::CreateAbilityFromDefinition(Definition, Owner);
	checkf(GrantedAbility, TEXT("GrantAbilityBattleEffect %s: failed to create granted ability"), *Config->GetName());
	Owner->GetAbilityInventory()->AddPassiveAbility(GrantedAbility);

	TArray<TInstancedStruct<FTacLogStepBase>> Steps;
	Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogAbilityGrantStep>(
		FTacLogAbilityGrantStep::Make(Owner->GetUnitID(), Definition->GetPrimaryAssetId(), GrantedAbility->GetAbilityId(), true)));
	LogActivation(MoveTemp(Steps));
}

void UGrantAbilityBattleEffect::OnRemoved()
{
	Owner->GetAbilityInventory()->RemovePassiveAbility(GrantedAbility);

	TArray<TInstancedStruct<FTacLogStepBase>> Steps;
	Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogAbilityGrantStep>(
		FTacLogAbilityGrantStep::Make(Owner->GetUnitID(), GrantedAbility->GetConfig()->GetPrimaryAssetId(), GrantedAbility->GetAbilityId(), false)));
	GrantedAbility = nullptr;
	LogRemoval(MoveTemp(Steps));
	Super::OnRemoved();
}
