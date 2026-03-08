#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTypes/TeamConstants.h"
#include "BattleTeam.generated.h"
class AUnit;

UCLASS()
class KBS_API UBattleTeam : public UObject
{
	GENERATED_BODY()
public:
	void AddUnit(AUnit* Unit);
	void RemoveUnit(AUnit* Unit);
	bool ContainsUnit(AUnit* Unit) const;
	void ClearUnits();
	ETeamSide GetTeamSide() const { return TeamSide; }
	void SetTeamSide(ETeamSide Side) { TeamSide = Side; }
	const TArray<TObjectPtr<AUnit>>& GetUnits() const { return Units; }
	bool IsAnyUnitAlive() const;
	bool IsAnyUnitOnField() const;
	bool IsOtherUnitAlive(AUnit* Unit) const;
	bool IsOtherUnitOnField(AUnit* Unit) const;
	// win condition check
	bool CanContinueFight() const;
	static ETeamSide ReverseTeamSide(ETeamSide Side);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team")
	ETeamSide TeamSide = ETeamSide::Attacker;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Team")
	TArray<TObjectPtr<AUnit>> Units;
};
