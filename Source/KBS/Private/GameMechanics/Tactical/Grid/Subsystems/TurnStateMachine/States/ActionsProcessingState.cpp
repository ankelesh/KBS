#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/ActionsProcessingState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/TacTurnOrder.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacCombatSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacAbilityExecutorService.h"
#include "GameMechanics/Tactical/Grid/Components/GridHighlightComponent.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameplayTypes/AbilityTypesLibrary.h"
#include "GameplayTypes/AbilityTypes.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacAICombatService.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"

void FActionsProcessingState::Enter()
{
	AUnit* CurrentUnit = GetTurnOrder()->GetCurrentUnit();
	if (UTacCombatSubsystem* CombatSubsystem = CurrentUnit->GetWorld()->GetSubsystem<UTacCombatSubsystem>())
	{
		ExecutorService = CombatSubsystem->GetAbilityExecutorService();
	}

	CheckAbilitiesAndSetupTurn();
}

void FActionsProcessingState::Exit()
{
	ExecutorService = nullptr;
	FTacTurnState::Exit();
}

void FActionsProcessingState::CellClicked(FTacCoordinates Cell)
{
	if (TurnProcessing == ETurnProcessingSubstate::EAwaitingInputState)
		ExecuteAbilityOnTarget(Cell);
}

void FActionsProcessingState::UnitClicked(AUnit* Unit)
{
	if (Unit && TurnProcessing == ETurnProcessingSubstate::EAwaitingInputState)
	{
		if (UTacGridSubsystem* GridSubsystem = Unit->GetWorld()->GetSubsystem<UTacGridSubsystem>(); GridSubsystem)
		{
			FTacCoordinates UnitCell;
			if (GridSubsystem->GetUnitCoordinates(Unit, UnitCell))
			{
				ExecuteAbilityOnTarget(UnitCell);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ActionsProcessingState: Failed to get coordinates for clicked unit"));
			}
		}
	}
}

void FActionsProcessingState::AbilityClicked(UUnitAbilityInstance* Ability)
{
	if (Ability && TurnProcessing == ETurnProcessingSubstate::EAwaitingInputState)
	{
		AUnit* CurrentUnit = GetTurnOrder()->GetCurrentUnit();
		CurrentUnit->GetAbilityInventory()->EquipAbility(Ability);
		CheckAbilitiesAndSetupTurn();
	}
}

void FActionsProcessingState::ExecuteAbilityOnTarget(FTacCoordinates TargetCell)
{
	if (!ExecutorService) return;

	AUnit* CurrentUnit = GetTurnOrder()->GetCurrentUnit();
	UUnitAbilityInstance* Ability = CurrentUnit->GetAbilityInventory()->GetCurrentActiveAbility();

	UE_LOG(LogKBSAI, Log, TEXT("ExecuteAbilityOnTarget: unit=%s ability=%s cell=[%d,%d]"),
		*CurrentUnit->GetName(), *Ability->GetAbilityDisplayData().AbilityName, TargetCell.Row, TargetCell.Col);

	FAbilityResult Result = ExecutorService->CheckAndExecute(Ability, TargetCell);

	if (Result.bInvalidInput)
	{
		UE_LOG(LogKBSAI, Warning, TEXT("  -> bInvalidInput — ability rejected target cell, turn may end silently"));
		return;
	}

	if (Result.bBattleEnded)
	{
		bBattleEnded = true;
		TurnProcessing = ETurnProcessingSubstate::EFreeState;
		if (UTacGridSubsystem* GridSubsystem = CurrentUnit->GetWorld()->GetSubsystem<UTacGridSubsystem>())
		{
			GridSubsystem->ClearAllHighlights();
		}
		return;
	}

	if (Result.bPresentationRunning)
	{
		UE_LOG(LogKBSAI, Log, TEXT("  -> bPresentationRunning — awaiting presentation"));
		TurnProcessing = ETurnProcessingSubstate::EAwaitingPresentationState;
	}
	else
	{
		UE_LOG(LogKBSAI, Log, TEXT("  -> no presentation, continuing turn"));
		CheckAbilitiesAndSetupTurn();
	}
}

bool FActionsProcessingState::IsAIUnit(AUnit* Unit) const
{
	UTacGridSubsystem* GridSubsystem = Unit->GetWorld()->GetSubsystem<UTacGridSubsystem>();
	return !GridSubsystem->GetPlayerTeam()->ContainsUnit(Unit);
}

void FActionsProcessingState::HandleAITurn(AUnit* Unit)
{
	UE_LOG(LogKBSTurn, Log, TEXT("AI turn for: %s"), *Unit->GetName());
	UTacAICombatService* AIService = ParentTurnSubsystem->GetAICombatService();
	FAiDecision Decision = AIService->ThinkOverNextAction(Unit);

	if (Decision.bHasDecision)
	{
		Unit->GetAbilityInventory()->EquipAbility(Decision.AbilityToUse);
		ExecuteAbilityOnTarget(Decision.TargetCell);
	}
	else
	{
		UE_LOG(LogKBSTurn, Warning, TEXT("AI has no decision for %s — ending turn"), *Unit->GetName());
		TurnProcessing = ETurnProcessingSubstate::EFreeState;
	}
}

void FActionsProcessingState::CheckAbilitiesAndSetupTurn()
{
	AUnit* CurrentUnit = GetTurnOrder()->GetCurrentUnit();
	UAbilityInventoryComponent* Inventory = CurrentUnit->GetAbilityInventory();
	UTacGridSubsystem* GridSubsystem = CurrentUnit->GetWorld()->GetSubsystem<UTacGridSubsystem>();

	GridSubsystem->ClearAllHighlights();
	Inventory->RecheckContents();

	if (IsAIUnit(CurrentUnit))
	{
		HandleAITurn(CurrentUnit);
		return;
	}

	if (bool res = Inventory->HasAnyAbilityAvailable())
	{
		
		Inventory->EnsureValidAbility();
		UUnitAbilityInstance* CurrentAbility = Inventory->GetCurrentActiveAbility();

		UTacGridTargetingService* TargetingService = GridSubsystem->GetGridTargetingService();
		ETargetReach AbilityTargeting = CurrentAbility->GetTargeting();
		TArray<FTacCoordinates> ValidCells = TargetingService->GetValidTargetCells(CurrentUnit, AbilityTargeting);
		EHighlightType HighlightType = UAbilityTypesLibrary::TargetReachToHighlightType(AbilityTargeting);

		GridSubsystem->ShowHighlights(ValidCells, HighlightType);
		TurnProcessing = ETurnProcessingSubstate::EAwaitingInputState;
	}
	else
	{
		TurnProcessing = ETurnProcessingSubstate::EFreeState;
	}
}

void FActionsProcessingState::OnPresentationComplete()
{
	if (TurnProcessing == ETurnProcessingSubstate::EAwaitingPresentationState)
	{
		CheckAbilitiesAndSetupTurn();
	}
}

ETurnState FActionsProcessingState::NextState()
{
	if (bBattleEnded)
	{
		return ETurnState::EBattleEndState;
	}
	return ETurnState::ETurnEndState;
}
