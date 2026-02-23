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

bool USummonSpellAbility::Execute(FTacCoordinates TargetCell)
{
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
	if (!NewUnit) return false;

	USummonedPassiveAbility* Passive = NewObject<USummonedPassiveAbility>(this);
	Passive->InitAsSummonedPassive(NewUnit, Owner,
		SummonConfig->bDespawnOnCasterDeath, SummonConfig->SummonDurationTurns);
	NewUnit->GetAbilityInventory()->AddPassiveAbility(Passive);

	ActiveSummon = NewUnit;
	Owner->GetStats().Status.SetFocus();
	ConsumeCharge();
	return true;
}

bool USummonSpellAbility::CanExecute(FTacCoordinates TargetCell) const
{
	if (RemainingCharges <= 0 || IsOutsideFocus()) return false;
	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return false;
	return TargetingService->HasValidTargetAtCell(Owner, TargetCell, ETargetReach::EmptyCell);
}

bool USummonSpellAbility::CanExecute() const
{
	if (RemainingCharges <= 0 || IsOutsideFocus()) return false;
	UTacGridTargetingService* TargetingService = GetTargetingService();
	if (!TargetingService) return false;
	return TargetingService->HasAnyValidTargets(Owner, ETargetReach::EmptyCell);
}

ETargetReach USummonSpellAbility::GetTargeting() const
{
	return ETargetReach::EmptyCell;
}

void USummonSpellAbility::DespawnActiveSummon()
{
	ActiveSummon->HandleDeath();
	ActiveSummon.Reset();
}
