#include "GameMechanics/Units/Abilities/UnitMovementAbility.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridMovementComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridTargetingComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Units/Unit.h"
#include "GameplayTypes/AbilityBattleContext.h"
#include "GameplayTypes/DamageTypes.h"

ETargetReach UUnitMovementAbility::GetTargeting() const
{
	return ETargetReach::Movement;
}

FAbilityResult UUnitMovementAbility::ApplyAbilityEffect(const FAbilityBattleContext& Context)
{
	if (!Context.SourceUnit || !Context.Grid)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom,
			FText::FromString("Invalid source unit or grid"));
	}

	if (Context.TargetCell.X < 0 || Context.TargetCell.Y < 0)
	{
		return CreateFailureResult(EAbilityFailureReason::InvalidTarget,
			FText::FromString("Invalid target cell"));
	}

	UGridMovementComponent* MovementComponent = Context.Grid->FindComponentByClass<UGridMovementComponent>();
	if (!MovementComponent)
	{
		return CreateFailureResult(EAbilityFailureReason::Custom,
			FText::FromString("Grid has no movement component"));
	}

	bool bMoveSuccess = MovementComponent->MoveUnit(Context.SourceUnit,
		Context.TargetCell.Y, Context.TargetCell.X);

	if (bMoveSuccess)
	{
		FAbilityResult Result = CreateSuccessResult();
		Result.TurnAction = EAbilityTurnAction::EndTurn;
		Result.UnitsAffected.Add(Context.SourceUnit);
		return Result;
	}

	return CreateFailureResult(EAbilityFailureReason::Custom,
		FText::FromString("Movement failed"));
}

FAbilityValidation UUnitMovementAbility::CanExecute(const FAbilityBattleContext& Context) const
{
	if (!Context.SourceUnit)
	{
		return FAbilityValidation::Failure(EAbilityFailureReason::Custom,
			FText::FromString("No source unit"));
	}

	if (!Context.Grid)
	{
		return FAbilityValidation::Failure(EAbilityFailureReason::Custom,
			FText::FromString("No grid reference"));
	}

	if (Context.TargetCell.X < 0 || Context.TargetCell.Y < 0)
	{
		return FAbilityValidation::Failure(EAbilityFailureReason::InvalidTarget,
			FText::FromString("No target cell specified"));
	}

	UGridTargetingComponent* TargetingComponent = Context.Grid->FindComponentByClass<UGridTargetingComponent>();
	if (!TargetingComponent)
	{
		return FAbilityValidation::Failure(EAbilityFailureReason::Custom,
			FText::FromString("No targeting component"));
	}

	TArray<FIntPoint> ValidCells = TargetingComponent->GetValidTargetCells(Context.SourceUnit, ETargetReach::Movement);
	if (!ValidCells.Contains(Context.TargetCell))
	{
		return FAbilityValidation::Failure(EAbilityFailureReason::OutOfRange,
			FText::FromString("Target cell not in valid movement range"));
	}

	return FAbilityValidation::Success();
}
