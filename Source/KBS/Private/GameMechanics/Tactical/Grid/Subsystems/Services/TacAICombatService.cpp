// Fill out your copyright notice in the Description page of Project Settings.
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacAICombatService.h"
DEFINE_LOG_CATEGORY(LogKBSAI);
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameplayTypes/AbilityTypes.h"

void UTacAICombatService::Initialize(UTacGridSubsystem* InGridSubsystem, UTacCombatSubsystem* InCombatSubsystem)
{
	checkf(InGridSubsystem, TEXT("TacAICombatService: GridSubsystem must not be null"));
	checkf(InCombatSubsystem, TEXT("TacAICombatService: CombatSubsystem must not be null"));
	GridSubsystem = InGridSubsystem;
	CombatSubsystem = InCombatSubsystem;
}

FAiDecision UTacAICombatService::ThinkOverNextAction(AUnit* Unit) const
{
	UE_LOG(LogKBSAI, Log, TEXT("AI thinking for unit: %s"), *Unit->GetName());
	FAiDecision Decision;
	if (TryDecideAttack(Unit, Decision)) return Decision;
	if (TryDecideMove(Unit, Decision)) return Decision;
	DecideWait(Unit, Decision);
	if (!Decision.bHasDecision)
		UE_LOG(LogKBSAI, Warning, TEXT("AI found no valid action for %s — turn will end immediately"), *Unit->GetName());
	return Decision;
}

bool UTacAICombatService::TryDecideAttack(AUnit* Unit, FAiDecision& OutDecision) const
{
	UUnitAbilityInstance* AttackAbility = Unit->GetAbilityInventory()->GetDefaultAbility(EDefaultAbilitySlot::Attack);
	if (!AttackAbility)
	{
		UE_LOG(LogKBSAI, Log, TEXT("  [Attack] No default attack ability"));
		return false;
	}
	if (!AttackAbility->CanExecute())
	{
		UE_LOG(LogKBSAI, Log, TEXT("  [Attack] '%s' CanExecute=false (no charges or blocked)"), *AttackAbility->GetAbilityDisplayData().AbilityName);
		return false;
	}

	TArray<FTacCoordinates> ValidCells = GridSubsystem->GetGridTargetingService()->GetValidTargetCells(Unit, AttackAbility->GetTargeting());
	if (ValidCells.IsEmpty())
	{
		UE_LOG(LogKBSAI, Log, TEXT("  [Attack] '%s' has no valid target cells"), *AttackAbility->GetAbilityDisplayData().AbilityName);
		return false;
	}

	OutDecision.AbilityToUse = AttackAbility;
	OutDecision.TargetCell = ValidCells[0];  // Targeting service returns closest-first
	OutDecision.bHasDecision = true;
	UE_LOG(LogKBSAI, Log, TEXT("  [Attack] decided '%s' -> cell [%d,%d]"), *AttackAbility->GetAbilityDisplayData().AbilityName, ValidCells[0].Row, ValidCells[0].Col);
	return true;
}

bool UTacAICombatService::TryDecideMove(AUnit* Unit, FAiDecision& OutDecision) const
{
	UUnitAbilityInstance* MoveAbility = Unit->GetAbilityInventory()->GetDefaultAbility(EDefaultAbilitySlot::Move);
	if (!MoveAbility)
	{
		UE_LOG(LogKBSAI, Log, TEXT("  [Move] No default move ability"));
		return false;
	}
	if (!MoveAbility->CanExecute())
	{
		UE_LOG(LogKBSAI, Log, TEXT("  [Move] '%s' CanExecute=false"), *MoveAbility->GetAbilityDisplayData().AbilityName);
		return false;
	}

	TArray<FTacCoordinates> ValidCells = GridSubsystem->GetGridTargetingService()->GetValidTargetCells(Unit, MoveAbility->GetTargeting());
	if (ValidCells.IsEmpty())
	{
		UE_LOG(LogKBSAI, Log, TEXT("  [Move] '%s' has no valid cells"), *MoveAbility->GetAbilityDisplayData().AbilityName);
		return false;
	}

	OutDecision.AbilityToUse = MoveAbility;
	OutDecision.TargetCell = PickMoveTowardEnemy(Unit, ValidCells);
	OutDecision.bHasDecision = true;
	UE_LOG(LogKBSAI, Log, TEXT("  [Move] decided '%s' -> cell [%d,%d]"), *MoveAbility->GetAbilityDisplayData().AbilityName, OutDecision.TargetCell.Row, OutDecision.TargetCell.Col);
	return true;
}

void UTacAICombatService::DecideWait(AUnit* Unit, FAiDecision& OutDecision) const
{
	UUnitAbilityInstance* WaitAbility = Unit->GetAbilityInventory()->GetDefaultAbility(EDefaultAbilitySlot::Wait);
	if (!WaitAbility || !WaitAbility->CanExecute())
	{
		UE_LOG(LogKBSAI, Warning, TEXT("  [Wait] No usable wait ability"));
		return;
	}

	TArray<FTacCoordinates> ValidCells = GridSubsystem->GetGridTargetingService()->GetValidTargetCells(Unit, WaitAbility->GetTargeting());
	if (ValidCells.IsEmpty())
	{
		UE_LOG(LogKBSAI, Warning, TEXT("  [Wait] No valid cells for wait ability"));
		return;
	}

	OutDecision.AbilityToUse = WaitAbility;
	OutDecision.TargetCell = ValidCells[0];
	OutDecision.bHasDecision = true;
	UE_LOG(LogKBSAI, Log, TEXT("  [Wait] decided '%s'"), *WaitAbility->GetAbilityDisplayData().AbilityName);
}

FTacCoordinates UTacAICombatService::PickMoveTowardEnemy(AUnit* Unit, const TArray<FTacCoordinates>& MoveCells) const
{
	UBattleTeam* MyTeam = (Unit->GetTeamSide() == ETeamSide::Attacker)
		? GridSubsystem->GetAttackerTeam()
		: GridSubsystem->GetDefenderTeam();

	TArray<FTacCoordinates> EnemyCells;
	for (AUnit* Candidate : GridSubsystem->GetActiveUnits())
	{
		if (MyTeam->ContainsUnit(Candidate)) continue;
		FTacCoordinates EnemyCell;
		if (GridSubsystem->GetUnitCoordinates(Candidate, EnemyCell))
			EnemyCells.Add(EnemyCell);
	}

	if (EnemyCells.IsEmpty()) return MoveCells[0];

	FTacCoordinates BestCell = MoveCells[0];
	int32 BestDist = INT_MAX;
	for (const FTacCoordinates& MoveCell : MoveCells)
	{
		for (const FTacCoordinates& EnemyCell : EnemyCells)
		{
			int32 Dist = FMath::Abs(MoveCell.Row - EnemyCell.Row) + FMath::Abs(MoveCell.Col - EnemyCell.Col);
			if (Dist < BestDist)
			{
				BestDist = Dist;
				BestCell = MoveCell;
			}
		}
	}
	return BestCell;
}
