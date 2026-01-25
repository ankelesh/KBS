#include "GameMechanics/Units/Abilities/UnitMovementAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridMovementService.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacGridTargetingService.h"


ETargetReach UUnitMovementAbility::GetTargeting() const
{
	return ETargetReach::Movement;
}

FAbilityResult UUnitMovementAbility::ApplyAbilityEffect(AUnit* SourceUnit, FTacCoordinates TargetCell)
{
	UTacGridMovementService* MovementService = GetMovementService(SourceUnit);
	if (!MovementService)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom,
			FText::FromString("Movement service not available"));
	}

	FTacMovementVisualData OutVisuals;
	TOptional<FTacMovementVisualData> OutSwappedVisuals;
	bool bMoveSuccess = MovementService->MoveUnit(SourceUnit, TargetCell, OutVisuals, OutSwappedVisuals);

	if (bMoveSuccess)
	{
		FAbilityResult Result = CreateSuccessResult();
		Result.TurnAction = EAbilityTurnAction::EndTurn;
		Result.UnitsAffected.Add(SourceUnit);
		return Result;
	}

	return CreateFailureResult(EAbilityFailureReason::Custom,
		FText::FromString("Movement failed"));
}

FAbilityValidation UUnitMovementAbility::CanExecute(AUnit* SourceUnit, FTacCoordinates TargetCell) const
{
	UTacGridTargetingService* TargetingService = GetTargetingService(SourceUnit);
	if (!TargetingService)
	{
		return FAbilityValidation::Failure(EAbilityFailureReason::Custom,
			FText::FromString("Targeting service not available"));
	}

	TArray<FTacCoordinates> ValidCells = TargetingService->GetValidTargetCells(SourceUnit, ETargetReach::Movement);
	if (!ValidCells.Contains(TargetCell))
	{
		return FAbilityValidation::Failure(EAbilityFailureReason::OutOfRange,
			FText::FromString("Target cell not in valid movement range"));
	}

	return FAbilityValidation::Success();
}

