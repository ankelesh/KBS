#include "GameMechanics/Units/Abilities/Spells/SummonSpellAbility.h"
#include "GameMechanics/Units/Abilities/Spells/SummonSpellAbilityDefinition.h"
#include "GameMechanics/Units/Abilities/Passives/SummonedPassiveAbility.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Units/Unit.h"

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

	USummonedPassiveAbility* Passive = NewObject<USummonedPassiveAbility>(this);
	Passive->InitAsSummonedPassive(NewUnit, Owner,
		SummonConfig->bDespawnOnCasterDeath, SummonConfig->SummonDurationTurns);
	NewUnit->GetAbilityInventory()->AddPassiveAbility(Passive);

	ActiveSummon = NewUnit;
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

void USummonSpellAbility::DespawnActiveSummon()
{
	ActiveSummon->HandleDeath();
	ActiveSummon.Reset();
}
