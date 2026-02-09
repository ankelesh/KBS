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
	AUnit* CurrentUnit = GetTurnOrder()->GetCurrentUnit();
	UAbilityInventoryComponent* Inventory = CurrentUnit->GetAbilityInventory();
	
	Inventory->EnsureValidAbility();

	UUnitAbilityInstance* CurrentAbility = Inventory->GetCurrentActiveAbility();
	UTacGridSubsystem* GridSubsystem = CurrentUnit->GetWorld()->GetSubsystem<UTacGridSubsystem>();
	UTacGridTargetingService* TargetingService = GridSubsystem->GetGridTargetingService();
	ETargetReach AbilityTargeting = CurrentAbility->GetTargeting();
	TArray<FTacCoordinates> ValidCells = TargetingService->GetValidTargetCells(CurrentUnit, AbilityTargeting);

	EHighlightType HighlightType = UAbilityTypesLibrary::TargetReachToHighlightType(AbilityTargeting);
	GridSubsystem->ShowHighlights(ValidCells, HighlightType);
}

void FActionsProcessingState::Exit()
{
	FTacTurnState::Exit();
}

void FActionsProcessingState::CellClicked(FTacCoordinates Cell)
{
	if (TurnProcessing != ETurnProcessingSubstate::EAwaitingInputState)
	{
		return;
	}

	ExecuteAbilityOnTarget(Cell);
}

void FActionsProcessingState::UnitClicked(AUnit* Unit)
{
	if (!Unit || TurnProcessing != ETurnProcessingSubstate::EAwaitingInputState)
	{
		return;
	}

	// Get unit's coordinates
	UTacGridSubsystem* GridSubsystem = Unit->GetWorld()->GetSubsystem<UTacGridSubsystem>();
	FTacCoordinates UnitCell;
	if (!GridSubsystem->GetUnitCoordinates(Unit, UnitCell))
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionsProcessingState: Failed to get coordinates for clicked unit"));
		return;
	}

	ExecuteAbilityOnTarget(UnitCell);
}

void FActionsProcessingState::AbilityClicked(UUnitAbilityInstance* Ability)
{
	if (!Ability || TurnProcessing != ETurnProcessingSubstate::EAwaitingInputState)
	{
		return;
	}

	AUnit* CurrentUnit = GetTurnOrder()->GetCurrentUnit();
	CurrentUnit->GetAbilityInventory()->EquipAbility(Ability);
	RefreshForNextAction();
}

void FActionsProcessingState::ExecuteAbilityOnTarget(FTacCoordinates TargetCell)
{
	AUnit* CurrentUnit = GetTurnOrder()->GetCurrentUnit();
	UUnitAbilityInstance* Ability = CurrentUnit->GetAbilityInventory()->GetCurrentActiveAbility();

	FAbilityResult Result = Ability->TriggerAbility(CurrentUnit, TargetCell);

	if (!Result.bSuccess)
	{
		// Stay in awaiting input, keep highlights
		return;
	}

	// Check if presentation is running
	UPresentationSubsystem* PresentationSys = UPresentationSubsystem::Get(CurrentUnit);
	if (PresentationSys && !PresentationSys->IsIdle())
	{
		// Wait for presentation to complete
		PendingTurnAction = Result.TurnAction;
		TurnProcessing = ETurnProcessingSubstate::EAwaitingPresentationState;
		// TurnSubsystem will call OnPresentationComplete when ready
	}
	else
	{
		// No presentation, handle immediately
		HandleAbilityComplete(Result.TurnAction);
	}
}

void FActionsProcessingState::RefreshForNextAction()
{
	AUnit* CurrentUnit = GetTurnOrder()->GetCurrentUnit();
	UAbilityInventoryComponent* Inventory = CurrentUnit->GetAbilityInventory();

	Inventory->EnsureValidAbility();

	UUnitAbilityInstance* CurrentAbility = Inventory->GetCurrentActiveAbility();
	UTacGridSubsystem* GridSubsystem = CurrentUnit->GetWorld()->GetSubsystem<UTacGridSubsystem>();
	UTacGridTargetingService* TargetingService = GridSubsystem->GetGridTargetingService();
	ETargetReach AbilityTargeting = CurrentAbility->GetTargeting();
	TArray<FTacCoordinates> ValidCells = TargetingService->GetValidTargetCells(CurrentUnit, AbilityTargeting);

	EHighlightType HighlightType = UAbilityTypesLibrary::TargetReachToHighlightType(AbilityTargeting);

	GridSubsystem->ClearAllHighlights();
	GridSubsystem->ShowHighlights(ValidCells, HighlightType);
}

void FActionsProcessingState::HandleAbilityComplete(EAbilityTurnAction TurnAction)
{
	if (TurnAction == EAbilityTurnAction::FreeTurn)
	{
		RefreshForNextAction();
		TurnProcessing = ETurnProcessingSubstate::EAwaitingInputState;
	}
	else // EndTurn or Wait
	{
		bTurnEnded = true;
		TurnProcessing = ETurnProcessingSubstate::EFreeState;

		AUnit* CurrentUnit = GetTurnOrder()->GetCurrentUnit();
		UTacGridSubsystem* GridSubsystem = CurrentUnit->GetWorld()->GetSubsystem<UTacGridSubsystem>();
		GridSubsystem->ClearAllHighlights();
	}
}

void FActionsProcessingState::OnPresentationComplete()
{
	if (TurnProcessing == ETurnProcessingSubstate::EAwaitingPresentationState)
	{
		HandleAbilityComplete(PendingTurnAction);
	}
}

ETurnProcessingSubstate FActionsProcessingState::CanReleaseState()
{
	return TurnProcessing;
}

ETurnState FActionsProcessingState::NextState()
{
	return ETurnState::ETurnEndState;
}
