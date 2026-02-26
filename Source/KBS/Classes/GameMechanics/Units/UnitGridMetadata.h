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

	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	EUnitOrientation Orientation;

	// Invalid when unit has no extra cell (1-cell unit, or 2-cell with no adjacent room).
	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	FTacCoordinates ExtraCell;

	// Cached from UUnitDefinition. Distinguishes "2-cell unit with no room" from "1-cell unit".
	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	int32 UnitSize;

	FUnitGridMetadata()
		: Coords(FTacCoordinates())
		, Team(ETeamSide::Attacker)
		, bOnField(false)
		, bOnFlank(false)
		, Orientation(EUnitOrientation::GridTop)
		, ExtraCell(FTacCoordinates::Invalid())
		, UnitSize(1)
		, bInitialized(false)
	{}

	FUnitGridMetadata(const FTacCoordinates& InCoords, ETeamSide InTeam, bool bInOnField, bool bInOnFlank,
		EUnitOrientation InOrientation, FTacCoordinates InExtraCell, int32 InUnitSize)
		: Coords(InCoords)
		, Team(InTeam)
		, bOnField(bInOnField)
		, bOnFlank(bInOnFlank)
		, Orientation(InOrientation)
		, ExtraCell(InExtraCell)
		, UnitSize(InUnitSize)
		, bInitialized(true)
	{}

	bool IsValid() const { return bInitialized; }
	bool IsOnField() const { return bOnField; }
	bool IsMultiCell() const { return UnitSize > 1; }
	bool HasExtraCell() const { return ExtraCell.IsValid(); }

	bool IsSameTeam(const FUnitGridMetadata& Other) const { return Team == Other.Team; }
	bool IsEnemy(const FUnitGridMetadata& Other) const { return Team != Other.Team; }
	bool IsAlly(const FUnitGridMetadata& Other) const { return Team == Other.Team; }

	static EUnitOrientation DefaultOrientationForTeam(ETeamSide Side)
	{
		return Side == ETeamSide::Defender ? EUnitOrientation::GridTop : EUnitOrientation::GridBottom;
	}

private:
	bool bInitialized;
};
