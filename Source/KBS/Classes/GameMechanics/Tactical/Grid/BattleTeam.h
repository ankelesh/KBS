#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BattleTeam.generated.h"
class AUnit;
UENUM(BlueprintType)
enum class ETeamSide : uint8
{
	Attacker UMETA(DisplayName = "Attacker"),
	Defender UMETA(DisplayName = "Defender")
};
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
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team")
	ETeamSide TeamSide = ETeamSide::Attacker;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Team")
	TArray<TObjectPtr<AUnit>> Units;
};
