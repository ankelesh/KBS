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

	FAbilityResult Result = ExecutorService->CheckAndExecute(Ability, TargetCell);

	if (Result.bInvalidInput)
	{
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
		TurnProcessing = ETurnProcessingSubstate::EAwaitingPresentationState;
	}
	else
	{
		CheckAbilitiesAndSetupTurn();
	}
}

void FActionsProcessingState::CheckAbilitiesAndSetupTurn()
{
	AUnit* CurrentUnit = GetTurnOrder()->GetCurrentUnit();
	UAbilityInventoryComponent* Inventory = CurrentUnit->GetAbilityInventory();
	UTacGridSubsystem* GridSubsystem = CurrentUnit->GetWorld()->GetSubsystem<UTacGridSubsystem>();

	GridSubsystem->ClearAllHighlights();

	if (Inventory->HasAnyAbilityAvailable())
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
