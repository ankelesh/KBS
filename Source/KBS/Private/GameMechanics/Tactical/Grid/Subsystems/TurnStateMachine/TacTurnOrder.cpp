#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/TacTurnOrder.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include <ranges>

FRolledInitiative::FRolledInitiative(int32 Base) : Rolled(0), Basic(Base), WaitModifier(WAIT_MODIFIER_ACTIVE)
{
}

FRolledInitiative::FRolledInitiative() : Rolled(0), Basic(0), WaitModifier(WAIT_MODIFIER_ACTIVE)
{
}

int32 FRolledInitiative::MakeRoll()
{
	Rolled = RollInitiative();
	return Rolled;
}

int32 FRolledInitiative::GetCurrent() const
{
	return (Rolled + Basic) * WaitModifier;
}

void FRolledInitiative::Wait()
{
	WaitModifier = WAIT_MODIFIER_DELAYED;
}

bool FRolledInitiative::CanWait() const
{
	return WaitModifier == WAIT_MODIFIER_ACTIVE;
}

void FRolledInitiative::UpdateBasic(int32 Base)
{
	Basic = Base;
}

int32 FRolledInitiative::RollInitiative()
{
	return FMath::RandRange(LOWER_INITIATIVE_ROLL, UPPER_INITIATIVE_ROLL);
}

bool FRolledInitiative::BreakTie(AUnit* UnitA, AUnit* UnitB, UBattleTeam* AttackerTeam)
{
	if (!AttackerTeam)
	{
		UE_LOG(LogTemp, Error, TEXT("Teamless tiebreak in TacTurnOrder!"));
		return false;
	}

	bool AIsAttacker = AttackerTeam->ContainsUnit(UnitA);
	bool BIsAttacker = AttackerTeam->ContainsUnit(UnitB);
	return AIsAttacker < BIsAttacker;
}


FTacTurnOrder::FTacTurnOrder() : Queue(), CurrentUnit(nullptr)
{
	Queue.Reserve(EXPECTED_BATTLE_SIZE);
}

int32 FTacTurnOrder::InitiativeRoll(AUnit* Unit)
{
	if (!Unit) return 0;

	const FGuid& UnitGuid = Unit->GetUnitID();
	if (!RolledInitiative.Find(UnitGuid))
	{
		FRolledInitiative rolled_initiative(Unit->GetStats().Initiative.GetValue());
		RolledInitiative.Add(UnitGuid, rolled_initiative);
	}
	return RolledInitiative[UnitGuid].MakeRoll();
}

void FTacTurnOrder::SortQueue()
{
	Queue.Sort([this](const TWeakObjectPtr<AUnit>& A, const TWeakObjectPtr<AUnit>& B)
	{
		AUnit* APtr = A.Get();
		AUnit* BPtr = B.Get();
		const FRolledInitiative* InitA = APtr ? RolledInitiative.Find(APtr->GetUnitID()) : nullptr;
		const FRolledInitiative* InitB = BPtr ? RolledInitiative.Find(BPtr->GetUnitID()) : nullptr;

		int32 ValueA = InitA ? InitA->GetCurrent() : 0;
		int32 ValueB = InitB ? InitB->GetCurrent() : 0;
		if (ValueA == ValueB)
			return FRolledInitiative::BreakTie(APtr, BPtr, AttackerTeam.Get());
		return ValueA < ValueB;
	});
}

AUnit* FTacTurnOrder::GetCurrentUnit()
{
	return CurrentUnit.Get();
}

TArray<AUnit*> FTacTurnOrder::GetRemainingUnits(int32 TruncList) const
{
	if (TruncList > 0)
	{
		TArray<AUnit*> RemainingUnits;
		RemainingUnits.Reserve(TruncList);
		for (int32 i = Queue.Num() - 1;
		     i >= 0 && RemainingUnits.Num() < TruncList; --i)
		{
			RemainingUnits.Add(Queue[i].Get());
		}
		return RemainingUnits;
	}
	else
	{
		TArray<AUnit*> RemainingUnits;
		RemainingUnits.Reserve(Queue.Num());
		for (const TWeakObjectPtr<AUnit>& Unit : Queue)
		{
			RemainingUnits.Add(Unit.Get());
		}
		Algo::Reverse(RemainingUnits);
		return RemainingUnits;
	}
}

int32 FTacTurnOrder::GetUnitInitiative(AUnit* Unit) const
{
	if (!Unit) return 0;
	const FRolledInitiative* Initiative = RolledInitiative.Find(Unit->GetUnitID());
	return Initiative ? Initiative->GetCurrent() : 0;
}

AUnit* FTacTurnOrder::Advance()
{
	AUnit* PreviousUnit = CurrentUnit.Get();

	if (PreviousUnit)
	{
		RolledInitiative.Remove(PreviousUnit->GetUnitID());
	}

	if (Queue.Num() > 0)
	{
		CurrentUnit = Queue.Last();
		Queue.RemoveAt(Queue.Num() - 1);
	}
	else
	{
		CurrentUnit = nullptr;
	}

	return PreviousUnit;
}

void FTacTurnOrder::UpdateAndResort()
{
	for (const TWeakObjectPtr<AUnit>& Unit : Queue)
	{
		AUnit* UnitPtr = Unit.Get();
		if (UnitPtr)
		{
			RolledInitiative.FindOrAdd(UnitPtr->GetUnitID()).UpdateBasic(UnitPtr->GetStats().Initiative.GetValue());
		}
	}
	SortQueue();
}

void FTacTurnOrder::UpdateUnitAndResort(AUnit* Unit)
{
	if (Unit)
	{
		RolledInitiative.FindOrAdd(Unit->GetUnitID()).UpdateBasic(Unit->GetStats().Initiative.GetValue());
	}
	SortQueue();
}

void FTacTurnOrder::Repopulate(TArray<AUnit*> Units, UBattleTeam* Attackers)
{
	Erase();
	Queue.Empty(Units.Num());
	for (AUnit* Unit : Units)
	{
		Queue.Add(Unit);
	}
	AttackerTeam = Attackers;
	for (const TWeakObjectPtr<AUnit>& Unit : Queue)
	{
		InitiativeRoll(Unit.Get());
	}
	SortQueue();

	CurrentUnit = nullptr;
}

void FTacTurnOrder::Erase()
{
	Queue.Empty();
	CurrentUnit = nullptr;
	RolledInitiative.Empty();
}

void FTacTurnOrder::Wait()
{
	AUnit* CurrentUnitPtr = CurrentUnit.Get();
	if (!CurrentUnitPtr) return;
	FRolledInitiative& ini = RolledInitiative.FindOrAdd(CurrentUnitPtr->GetUnitID());
	if (ini.CanWait())
	{
		Queue.Add(CurrentUnit);
		ini.Wait();
		SortQueue();
	}
}

void FTacTurnOrder::InsertUnit(AUnit* Unit)
{
	if (!Unit) return;

	InitiativeRoll(Unit);
	Queue.Add(Unit);
	SortQueue();
}

void FTacTurnOrder::RemoveUnit(AUnit* Unit)
{
	if (!Unit) return;

	Queue.RemoveAll([Unit](const TWeakObjectPtr<AUnit>& Elem) { return Elem.Get() == Unit; });
	RolledInitiative.Remove(Unit->GetUnitID());

	if (CurrentUnit.Get() == Unit)
	{
		CurrentUnit = nullptr;
	}
}

bool FTacTurnOrder::Empty() const
{
	return !CurrentUnit.IsValid() && Queue.Num() == 0;
}

bool FTacTurnOrder::IsRoundEnd() const
{
	return Queue.Num() == 0;
}
