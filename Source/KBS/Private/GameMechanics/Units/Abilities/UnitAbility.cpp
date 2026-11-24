#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "GameMechanics/Units/Abilities/UnitAbilityDefinition.h"
#include "GameplayTypes/TargetingDescriptor.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridMovementService.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Components/UnitChargeComponent.h"
#include "GameMechanics/Units/Combat/Weapon.h"
#include "GameMechanics/Units/Combat/CombatDescriptor.h"
#include "GameMechanics/Units/Combat/CombatDescriptorData.h"
#include "GameMechanics/Units/BattleEffects/BattleEffect.h"
#include "GameMechanics/Tactical/DamageCalculation.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameplayTypes/AbilityTypesLibrary.h"

FGameplayTagContainer UUnitAbility::GetTags() const
{
	if (!bTagsCached)
	{
		CachedTags = BuildTags();
		bTagsCached = true;
	}
	return CachedTags;
}

FGameplayTagContainer UUnitAbility::BuildTags() const
{
	return Config ? Config->ExtraTags : FGameplayTagContainer{};
}

void UUnitAbility::InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner)
{
	Config = InDefinition;
	Owner = InOwner;
	if (Config)
	{
		RemainingUses = Config->MaxUses;
	}
}

bool UUnitAbility::RefreshAvailability() const
{
	bool bResult = CanExecute();
	OnAbilityAvailabilityChange.Broadcast(this, bResult);
	return bResult;
}

FTargetingDescriptor UUnitAbility::GetTargeting() const
{
	return FTargetingDescriptor::FromReach(Config->Targeting);
}

bool UUnitAbility::IsPassive() const
{
	if (Config)
	{
		return Config->bIsPassive;
	}
	return false;
}

void UUnitAbility::HandleTurnEnd()
{
	if (Config->UsesRefresh == EAbilityUsesRefresh::PerTurn)
		RestoreUses();
}

void UUnitAbility::ChangeSelection(bool bIsSelected)
{
	bIsCurrent = bIsSelected;
}

void UUnitAbility::ConsumeUse()
{
	if (RemainingUses > 0)
	{
		RemainingUses--;
	}
}

void UUnitAbility::RestoreUses()
{
	if (Config)
	{
		RemainingUses = Config->MaxUses;
	}
}

void UUnitAbility::BroadcastUsage() const
{
	OnAbilityUsed.Broadcast(RemainingUses, CanExecute());
}

FString UUnitAbility::GetDebugString() const
{
	if (!Config)
		return TEXT("<no config> [?/?] explicit:no");
	return FString::Printf(TEXT("%s [%d/%d] explicit:%s"),
		*Config->AbilityName, RemainingUses, Config->MaxUses,
		HasExplicitUses() ? TEXT("yes") : TEXT("no"));
}

FAbilityDisplayData UUnitAbility::GetAbilityDisplayData() const
{
	FAbilityDisplayData DisplayData;
	if (!Config)
	{
		return DisplayData;
	}
	DisplayData.AbilityName = Config->AbilityName;
	DisplayData.Icon = Config->Icon;
	DisplayData.RemainingUses = RemainingUses;
	DisplayData.MaxUses = Config->MaxUses;
	DisplayData.bIsEmpty = false;
	DisplayData.bCanExecuteThisTurn = CanExecute();
	DisplayData.TargetingInfo = UAbilityTypesLibrary::TargetReachToString(Config->Targeting);
	DisplayData.Description = FString::Printf(TEXT("%s - %s"), *Config->AbilityName, *DisplayData.TargetingInfo);
	return DisplayData;
}

UTacGridSubsystem* UUnitAbility::GetGridSubsystem() const
{
	if (!Owner) return nullptr;
	UWorld* World = Owner->GetWorld();
	if (!World) return nullptr;
	return World->GetSubsystem<UTacGridSubsystem>();
}

UTacGridMovementService* UUnitAbility::GetMovementService() const
{
	UTacGridSubsystem* GridSys = GetGridSubsystem();
	return GridSys ? GridSys->GetGridMovementService() : nullptr;
}

UTacGridTargetingService* UUnitAbility::GetTargetingService() const
{
	UTacGridSubsystem* GridSys = GetGridSubsystem();
	return GridSys ? GridSys->GetGridTargetingService() : nullptr;
}

UTacCombatSubsystem* UUnitAbility::GetCombatSubsystem() const
{
	if (!Owner) return nullptr;
	UWorld* World = Owner->GetWorld();
	if (!World) return nullptr;
	return World->GetSubsystem<UTacCombatSubsystem>();
}

UTacAbilityExecutorService* UUnitAbility::GetExecutorService() const
{
	UTacCombatSubsystem* CombatSubsystem = GetCombatSubsystem();
	return CombatSubsystem ? CombatSubsystem->GetAbilityExecutorService() : nullptr;
}

UTacTurnSubsystem* UUnitAbility::GetTurnSubsystem() const
{
	if (!Owner) return nullptr;
	UWorld* World = Owner->GetWorld();
	if (!World) return nullptr;
	return World->GetSubsystem<UTacTurnSubsystem>();
}

UTacLogSubsystem* UUnitAbility::GetLogSubsystem() const
{
	if (!Owner) return nullptr;
	UWorld* World = Owner->GetWorld();
	if (!World) return nullptr;
	return World->GetSubsystem<UTacLogSubsystem>();
}

FAbilityContext* UUnitAbility::GetContext() const
{
	if (!Owner) return nullptr;
	if (const auto Inventory = Owner->GetAbilityInventory())
		return Inventory->GetContext();
	return nullptr;
}

EAbilityTurnReleasePolicy UUnitAbility::DecideTurnRelease() const
{
	return Config->DefaultReleasePolicy;
}

bool UUnitAbility::CanActByContext() const
{
	FAbilityContext* Context = GetContext();
	checkf(Context, TEXT("Context empty on ability"));
		return Context->CanConditionalAct(bIsCurrent, Config->LookupTag.ConditionalTag,
		                                  Config->LookupTag.bIsPersistent);
}

bool UUnitAbility::OwnerCanAct() const
{
	return Owner->CanAct();
}

void UUnitAbility::SetCompletionTag() const
{
	if (Config->CompletionTag.IsValid())
		if (auto Context = GetContext())
			Context->AddTag(Config->CompletionTag.ConditionalTag, Config->CompletionTag.bIsPersistent);
}


void UUnitAbility::AppendChargeStep(TArray<TInstancedStruct<FTacLogStepBase>>& OutSteps, const FGameplayTag& ChargeTag,
                                    int32 Delta) const
{
	if (!ChargeTag.IsValid())
	{
		return;
	}
	OutSteps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogChargeStep>(
		FTacLogChargeStep::Make(Owner->GetUnitID(), ChargeTag, Delta, Owner->GetCharges().GetCharge(ChargeTag))));
}

void UUnitAbility::ApplyConfiguredEffects(AUnit* Target, const TArray<FDescriptorEffectConfig>& Effects,
                                          TArray<TInstancedStruct<FTacLogStepBase>>& OutSteps) const
{
	for (const FDescriptorEffectConfig& EffectConfig : Effects)
	{
		checkf(EffectConfig.EffectClass && EffectConfig.EffectConfig,
		       TEXT("%s: effect entry needs both class and config"), *Config->AbilityName);
		UBattleEffect* Effect = NewObject<UBattleEffect>(Target, EffectConfig.EffectClass);
		Effect->Initialize(EffectConfig.EffectConfig);
		Effect->PrepareForApply(Owner, Target);
		const EEffectApplicationOutcome Outcome = Target->ApplyEffect(Effect);
		OutSteps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogEffectSpawnStep>(
			FTacLogEffectSpawnStep::Make(Target->GetUnitID(),
			                             {Effect->GetConfig()->GetPrimaryAssetId(), Effect->GetEffectId(),
			                              Effect->GetDuration(), Outcome})));
	}
}

bool UUnitAbility::TryFireReactionAttack(AUnit* Target, bool bUseWeapon, UCombatDescriptor* OwnDescriptor,
                                         FGameplayTag OwnAnimTag)
{
	UTacCombatSubsystem* CombatSubsystem = GetCombatSubsystem();
	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	check(CombatSubsystem);
	check(LogSubsystem);

	UWeapon* Weapon = bUseWeapon ? FDamageCalculation::SelectWeaponForTarget(Owner, Target, true) : nullptr;
	UCombatDescriptor* Descriptor = Weapon ? Weapon->GetDescriptor() : OwnDescriptor;
	if (!Descriptor)
		return false;

	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::Ability, ETacLogEventOrigin::Reaction,
	                                        Owner->GetUnitID(), LogSubsystem->FindLatestEvent());
	TArray<FCombatHitResult> HitResults = CombatSubsystem->ResolveReactionAttack(Owner, {Target}, Descriptor);

	FAbilityUsePayload Payload;
	Payload.AbilityAssetId    = Config->GetPrimaryAssetId();
	Payload.AbilityInstanceId = AbilityId;
	Payload.Steps.Add(TInstancedStruct<FTacLogStepBase>::Make<FTacLogCombatStep>(
		FTacLogCombatStep::Make(Owner->GetUnitID(), Owner->GetGridMetadata().Coords, Target->GetUnitID(), HitResults,
		                        Weapon ? Weapon->GetAnimTag() : OwnAnimTag,
		                        Weapon ? Weapon->GetDeliveryManifestPath() : FSoftObjectPath())));
	FTacLogEffectSpawnStep::AppendFromHits(Payload.Steps, HitResults);
	FTacLogStatusChangeStep::AppendDefendingClearedFromHits(Payload.Steps, HitResults);
	if (Weapon)
	{
		const FUnitChargeCost PaidCost = Owner->PayWeaponCost(*Weapon);
		AppendChargeStep(Payload.Steps, PaidCost.Tag, -PaidCost.Amount);
	}
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FAbilityUsePayload>(MoveTemp(Payload)));
	return true;
}
