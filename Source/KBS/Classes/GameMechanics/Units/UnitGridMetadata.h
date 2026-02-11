#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "UnitGridMetadata.generated.h"

USTRUCT(BlueprintType)
struct KBS_API FUnitGridMetadata
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	FTacCoordinates Coords;

	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	ETeamSide Team;

	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	bool bOnField;

	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	bool bOnFlank;

	FUnitGridMetadata()
		: Coords(FTacCoordinates())
		, Team(ETeamSide::Attacker)
		, bOnField(false)
		, bOnFlank(false)
		, bInitialized(false)
	{}

	FUnitGridMetadata(const FTacCoordinates& InCoords, ETeamSide InTeam, bool bInOnField, bool bInOnFlank)
		: Coords(InCoords)
		, Team(InTeam)
		, bOnField(bInOnField)
		, bOnFlank(bInOnFlank)
		, bInitialized(true)
	{}

	bool IsValid() const { return bInitialized; }
	bool IsOnField() const { return bOnField; }

	bool IsSameTeam(const FUnitGridMetadata& Other) const { return Team == Other.Team; }
	bool IsEnemy(const FUnitGridMetadata& Other) const { return Team != Other.Team; }
	bool IsAlly(const FUnitGridMetadata& Other) const { return Team == Other.Team; }

private:
	bool bInitialized;
};
