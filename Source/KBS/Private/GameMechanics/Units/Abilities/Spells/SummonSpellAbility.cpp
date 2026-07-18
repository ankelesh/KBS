#include "GameMechanics/Units/Abilities/Spells/SummonSpellAbility.h"
#include "GameMechanics/Units/Abilities/Spells/SummonSpellAbilityDefinition.h"
#include "GameMechanics/Units/Abilities/Passives/SummonedPassiveAbility.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacLogSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/Tags/Tactical/AbilityTags.h"

void USummonSpellAbility::InitializeFromDefinition(UUnitAbilityDefinition* InDefinition, AUnit* InOwner)
{
	Super::InitializeFromDefinition(InDefinition, InOwner);
	SummonConfig = Cast<USummonSpellAbilityDefinition>(Config);
	checkf(SummonConfig, TEXT("USummonSpellAbility requires a USummonSpellAbilityDefinition asset"));
}

FAbilityExecutionResult USummonSpellAbility::Execute(FTacCoordinates TargetCell)
{
	check(Owner);
	if (SummonConfig->bReplacePreviousSummon && ActiveSummon.IsValid())
	{
		DespawnActiveSummon();
	}

	UTacGridSubsystem* GridSubsystem = GetGridSubsystem();
	checkf(GridSubsystem, TEXT("USummonSpellAbility::Execute: GridSubsystem unavailable"));

	UBattleTeam* OwnerTeam = Owner->GetTeamSide() == ETeamSide::Attacker
		? GridSubsystem->GetAttackerTeam()
		: GridSubsystem->GetDefenderTeam();

	AUnit* NewUnit = GridSubsystem->SpawnSummonedUnit(
		SummonConfig->SummonedUnitClass,
		SummonConfig->SummonedUnitDefinition,
		TargetCell,
		OwnerTeam
	);
	if (!NewUnit) return FAbilityExecutionResult::MakeFail();

	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	FGuid SpawnEventId;
	if (LogSubsystem)
	{
		FString TypeName = SummonConfig->SummonedUnitDefinition
			? SummonConfig->SummonedUnitDefinition->UnitName : TEXT("Unit");
		LogSubsystem->RegisterUnit(NewUnit->GetUnitID(), TypeName);
		FGuid ParentId = LogSubsystem->FindClosestEvent(ETacLogEventType::Ability);
		SpawnEventId = LogSubsystem->OpenEvent(ETacLogEventType::UnitSpawn,
			ETacLogEventOrigin::Triggered, Owner->GetUnitID(), ParentId);
	}

	USummonedPassiveAbility* Passive = NewObject<USummonedPassiveAbility>(this);
	Passive->InitAsSummonedPassive(NewUnit, Owner,
		SummonConfig->bDespawnOnCasterDeath, SummonConfig->SummonDurationTurns);
	NewUnit->GetAbilityInventory()->AddPassiveAbility(Passive);

	ActiveSummon = NewUnit;

	if (LogSubsystem && SpawnEventId.IsValid())
	{
		FUnitSpawnPayload Payload;
		Payload.SpawnedUnitId  = NewUnit->GetUnitID();
		Payload.SpawnCoords    = TargetCell;
		Payload.TeamSide       = OwnerTeam->GetTeamSide();
		Payload.SummonerUnitId = Owner->GetUnitID();
		Payload.bIsSummon      = true;
		if (SummonConfig->SummonedUnitDefinition)
			Payload.UnitDefinitionId = SummonConfig->SummonedUnitDefinition->GetPrimaryAssetId();
		LogSubsystem->CloseEvent(SpawnEventId,
			TInstancedStruct<FTacLogPayload>::Make<FUnitSpawnPayload>(MoveTemp(Payload)));
	}

	ConsumeCharge();
	SetCompletionTag();
	return FAbilityExecutionResult::MakeOk(DecideTurnRelease());
}

bool USummonSpellAbility::CanExecute(FTacCoordinates TargetCell) const
{
	if (RemainingCharges <= 0 || !OwnerCanAct() || !CanActByContext()) return false;
	UTacGridTargetingService* TargetingService = GetTargetingService();
	check(TargetingService);
	return TargetingService->HasValidTargetAtCell(Owner, TargetCell, GetTargeting());
}

bool USummonSpellAbility::CanExecute() const
{
	if (RemainingCharges <= 0 || !OwnerCanAct() || !CanActByContext()) return false;
	UTacGridTargetingService* TargetingService = GetTargetingService();
	check(TargetingService);
	return TargetingService->HasAnyValidTargets(Owner, GetTargeting());
}

FTargetingDescriptor USummonSpellAbility::GetTargeting() const
{
	return FTargetingDescriptor::FromReach(ETargetReach::EmptyCell);
}

FGameplayTagContainer USummonSpellAbility::BuildTags() const
{
	FGameplayTagContainer Tags = Super::BuildTags();
	Tags.AddTag(TAG_ABILITY_SPELL);
	Tags.AddTag(TAG_ABILITY_SPELL_SUMMON);
	return Tags;
}

void USummonSpellAbility::DespawnActiveSummon()
{
	UTacLogSubsystem* LogSubsystem = GetLogSubsystem();
	check(LogSubsystem);
	FGuid EventId = LogSubsystem->OpenEvent(ETacLogEventType::UnitDespawn, ETacLogEventOrigin::Triggered,
	                                        ActiveSummon->GetUnitID(), FGuid());

	const ETeamSide TeamSide = ActiveSummon->GetTeamSide();
	const FGuid UnitId = ActiveSummon->GetUnitID();
	GetGridSubsystem()->DespawnUnit(ActiveSummon.Get());

	FUnitDespawnPayload Payload;
	Payload.UnitId   = UnitId;
	Payload.TeamSide = TeamSide;
	Payload.Reason   = EUnitDespawnReason::Replaced;
	LogSubsystem->CloseEvent(EventId, TInstancedStruct<FTacLogPayload>::Make<FUnitDespawnPayload>(MoveTemp(Payload)));

	ActiveSummon.Reset();
}
