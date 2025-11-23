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

	ETeamSide GetTeamSide() const { return TeamSide; }
	void SetTeamSide(ETeamSide Side) { TeamSide = Side; }

	const TSet<TObjectPtr<AUnit>>& GetUnits() const { return Units; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team")
	ETeamSide TeamSide = ETeamSide::Attacker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Team")
	TSet<TObjectPtr<AUnit>> Units;
};
