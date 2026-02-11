#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/ActionsProcessingState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/TacTurnOrder.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"
#include "GameMechanics/Tactical/Grid/Components/GridHighlightComponent.h"
#include "GameMechanics/Tactical/PresentationSubsystem.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameplayTypes/AbilityTypesLibrary.h"
#include "GameplayTypes/AbilityTypes.h"

void FActionsProcessingState::Enter()
{
	CheckAbilitiesAndSetupTurn();
}

void FActionsProcessingState::Exit()
{
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
	AUnit* CurrentUnit = GetTurnOrder()->GetCurrentUnit();
	UUnitAbilityInstance* Ability = CurrentUnit->GetAbilityInventory()->GetCurrentActiveAbility();
	bool Result = Ability->Execute(TargetCell);

	if (!Result)
	{
		// Stay in awaiting input, keep highlights
		return;
	}

	// Check win condition after ability execution
	if (CheckWinCondition())
	{
		bBattleEnded = true;
		TurnProcessing = ETurnProcessingSubstate::EFreeState;
		UTacGridSubsystem* GridSubsystem = CurrentUnit->GetWorld()->GetSubsystem<UTacGridSubsystem>();
		GridSubsystem->ClearAllHighlights();
		return;
	}

	// Check if presentation is running
	if (UPresentationSubsystem* PresentationSys = UPresentationSubsystem::Get(CurrentUnit); PresentationSys && !PresentationSys->IsIdle())
	{
		// Wait for presentation to complete
		TurnProcessing = ETurnProcessingSubstate::EAwaitingPresentationState;
	}
	else
	{
		// No presentation, loop back to ability check
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
