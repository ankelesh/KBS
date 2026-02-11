#pragma once
#include "CoreMinimal.h"


class AUnit;
class UBattleTeam;
struct FRolledInitiative
{
	int32 Rolled;
	int32 Basic;
	int32 WaitModifier;
	FRolledInitiative(int32 Base);
	FRolledInitiative();
	int32 MakeRoll();
	int32 GetCurrent() const;
	void Wait();
	bool CanWait() const;
	void UpdateBasic(int32 Base);

	static int32 RollInitiative();
	static bool BreakTie(AUnit* UnitA, AUnit* UnitB, UBattleTeam* AttackerTeam); 
	static constexpr int32 LOWER_INITIATIVE_ROLL = -4;
	static constexpr int32 UPPER_INITIATIVE_ROLL = 4;
	static constexpr int32 WAIT_MODIFIER_ACTIVE = 1;
	static constexpr int32 WAIT_MODIFIER_DELAYED = -1;
};

class FTacTurnOrder
{
public:
	FTacTurnOrder();
	AUnit* GetCurrentUnit();
	// returns up to Trunc units from queue except current
	TArray<AUnit*> GetRemainingUnits(int32 TruncList=-1) const;
	int32 GetUnitInitiative(AUnit* Unit) const;
	// returns previous current unit, pops queue and replaces current unit
	AUnit* Advance();
	// Resorts queue by Unit ModifiedStats->Initiative
	void UpdateAndResort();
	void UpdateUnitAndResort(AUnit* Unit);
	// Erases state and populates Queue. Does not advance turn, sets up CurrentUnit to nullptr
	void Repopulate(TArray<AUnit*> Units, UBattleTeam* Attackers);
	void Erase();
	// shuffles unit back into queue, reverting it's initiative to (- initiative) (only for sorting)
	void Wait();
	// inserts unit into turn order (for summoned units), rolls initiative and resorts without rerolling existing units
	void InsertUnit(AUnit* Unit);
	void RemoveUnit(AUnit* Unit);
	// is true when current unit is nullptr and queue size is 0
	bool Empty() const;
	bool IsRoundEnd() const;
private:
	int32 InitiativeRoll(AUnit* Unit);
	void SortQueue();

	static constexpr int32 EXPECTED_BATTLE_SIZE = 30;

	TArray<TWeakObjectPtr<AUnit>> Queue;
	TWeakObjectPtr<AUnit> CurrentUnit;
	TMap<FGuid, FRolledInitiative> RolledInitiative;
	TWeakObjectPtr<UBattleTeam> AttackerTeam;
};
