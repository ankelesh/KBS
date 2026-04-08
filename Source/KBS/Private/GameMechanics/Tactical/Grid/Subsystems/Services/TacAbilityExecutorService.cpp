// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMechanics/Tactical/Grid/Subsystems/Services/TacAbilityExecutorService.h"
#include "GameplayTypes/AbilityTypes.h"
#include "GameMechanics/Units/Abilities/UnitAbility.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/PresentationSubsystem.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameplayTypes/GridCoordinates.h"

FAbilityResult UTacAbilityExecutorService::CheckAndExecute(UUnitAbility* Ability, FTacCoordinates TargetCell)
{
	check(Ability);

	if (!Ability->CanExecute(TargetCell))
	{
		FAbilityResult Result;
		Result.bInvalidInput = true;
		return Result;
	}

	OnAbilityExecuting.Broadcast(Ability, TargetCell);
	return Execute(Ability, TargetCell);
}

FAbilityResult UTacAbilityExecutorService::Execute(UUnitAbility* Ability, FTacCoordinates TargetCell)
{
	check(Ability);
	AUnit* Owner = Ability->GetOwner();
	check(Owner);
	auto Context = Owner->GetAbilityInventory()->GetContext();
	check(Context);
	FAbilityResult Result;

	FAbilityExecutionResult ExecutionResult = Ability->Execute(TargetCell);
	if (!ExecutionResult.IsOk())
	{
		Result.bInvalidInput = true;
		return Result;
	}
	else
	{
		Owner->GetAbilityInventory()->ProcessTurnPolicy(ExecutionResult.Policy);
	}

	// Check win condition
	if (UTacGridSubsystem* GridSubsystem = Owner->GetWorld()->GetSubsystem<UTacGridSubsystem>())
	{
		Result.bBattleEnded = !GridSubsystem->IsBothTeamsAnyUnitAlive();
	}

	// Check presentation status
	if (UPresentationSubsystem* PresentationSys = UPresentationSubsystem::Get(Owner))
	{
		Result.bPresentationRunning = !PresentationSys->IsIdle();
	}

	
	OnAbilityCompleted.Broadcast(Ability, TargetCell, Result);
	Ability->BroadcastUsage();
	return Result;
}
