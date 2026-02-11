#include "GameMechanics/Units/Abilities/UnitDefendAbility.h"
#include "GameMechanics/Units/Unit.h"


bool UUnitDefendAbility::Execute(FTacCoordinates TargetCell)
{
	if (!Owner) return false;

	Owner->SetDefending(true);
	UE_LOG(LogTemp, Log, TEXT("%s is now defending - incoming damage will be halved"), *Owner->GetName());

	// Set Focused status to end turn
	Owner->GetStats().Status.SetFocus();
	ConsumeCharge();

	return true;
}

bool UUnitDefendAbility::CanExecute(FTacCoordinates TargetCell) const
{
	return Owner && RemainingCharges > 0;
}

bool UUnitDefendAbility::CanExecute() const
{
	return Owner && RemainingCharges > 0;
}

void UUnitDefendAbility::Subscribe()
{
	if (Owner)
	{
		Owner->OnUnitTurnEnd.AddDynamic(this, &UUnitDefendAbility::HandleTurnEnd);
	}
}

void UUnitDefendAbility::Unsubscribe()
{
	if (Owner)
	{
		Owner->OnUnitTurnEnd.RemoveDynamic(this, &UUnitDefendAbility::HandleTurnEnd);
	}
}

void UUnitDefendAbility::HandleTurnEnd(AUnit*)
{
	RestoreCharges();
}
