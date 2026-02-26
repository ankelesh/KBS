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

	EUnitOrientation ToAbsoluteOrientation(ETeamRelativeDir Dir) const
	{
		// Defender faces GridTop; Attacker faces GridBottom (looking south flips Left<->Right).
		const bool bDefender = (Team == ETeamSide::Defender);
		switch (Dir)
		{
		case ETeamRelativeDir::Front: return bDefender ? EUnitOrientation::GridTop    : EUnitOrientation::GridBottom;
		case ETeamRelativeDir::Back:  return bDefender ? EUnitOrientation::GridBottom : EUnitOrientation::GridTop;
		case ETeamRelativeDir::Left:  return bDefender ? EUnitOrientation::GridLeft   : EUnitOrientation::GridRight;
		case ETeamRelativeDir::Right: return bDefender ? EUnitOrientation::GridRight  : EUnitOrientation::GridLeft;
		}
		return EUnitOrientation::GridTop;
	}

	ETeamRelativeDir ToTeamRelativeDir(EUnitOrientation AbsOrientation) const
	{
		const bool bDefender = (Team == ETeamSide::Defender);
		if (bDefender)
		{
			switch (AbsOrientation)
			{
			case EUnitOrientation::GridTop:    return ETeamRelativeDir::Front;
			case EUnitOrientation::GridBottom: return ETeamRelativeDir::Back;
			case EUnitOrientation::GridLeft:   return ETeamRelativeDir::Left;
			case EUnitOrientation::GridRight:  return ETeamRelativeDir::Right;
			}
		}
		else
		{
			switch (AbsOrientation)
			{
			case EUnitOrientation::GridBottom: return ETeamRelativeDir::Front;
			case EUnitOrientation::GridTop:    return ETeamRelativeDir::Back;
			case EUnitOrientation::GridRight:  return ETeamRelativeDir::Left;
			case EUnitOrientation::GridLeft:   return ETeamRelativeDir::Right;
			}
		}
		return ETeamRelativeDir::Front;
	}

private:
	bool bInitialized;
};
