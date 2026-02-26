#pragma once
#include "CoreMinimal.h"
#include "GridCoordinates.generated.h"


UENUM(BlueprintType)
enum class ETacGridLayer : uint8
{
	Ground = 0 UMETA(DisplayName = "Ground"),
	Air = 1 UMETA(DisplayName = "Air")
};

UENUM(BlueprintType)
enum class EUnitOrientation : uint8
{
	GridTop    = 0 UMETA(DisplayName = "Grid Top"),    // Facing row=0
	GridBottom = 1 UMETA(DisplayName = "Grid Bottom"), // Facing last row
	GridLeft   = 2 UMETA(DisplayName = "Grid Left"),   // Facing col=0
	GridRight  = 3 UMETA(DisplayName = "Grid Right")   // Facing col=4
};

UENUM(BlueprintType)
enum class ERelativeTurn : uint8
{
	Left  UMETA(DisplayName = "Turn Left"),
	Right UMETA(DisplayName = "Turn Right")
};

UENUM(BlueprintType)
enum class ETeamRelativeDir : uint8
{
	Front  UMETA(DisplayName = "Team Front"),  // Toward opponent's base
	Back   UMETA(DisplayName = "Team Back"),   // Toward own base
	Left   UMETA(DisplayName = "Team Left"),   // Left from unit's facing perspective
	Right  UMETA(DisplayName = "Team Right")   // Right from unit's facing perspective
};


USTRUCT(BlueprintType)
struct KBS_API FTacCoordinates
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Row;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Col;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETacGridLayer Layer;

	int32 X() const { return Row; }
	int32 Y() const { return Col; }
	ETacGridLayer Z() const { return Layer; }
	FTacCoordinates() : Row(0), Col(0), Layer(ETacGridLayer::Ground) {}
	FTacCoordinates(int32 Row, int32 Col) : Row(Row), Col(Col), Layer(ETacGridLayer::Ground) {}
	FTacCoordinates(int32 Row, int32 Col, ETacGridLayer Layer) : Row(Row), Col(Col), Layer(Layer) {}

	static FTacCoordinates Invalid() { return FTacCoordinates(-1, -1); }
	bool IsValid() const { return Row >= 0 && Col >= 0; }

	bool operator==(const FTacCoordinates& Other) const
	{
		return Row == Other.Row && Col == Other.Col && Layer == Other.Layer;
	}

	bool operator!=(const FTacCoordinates& Other) const
	{
		return !(*this == Other);
	}

	bool IsValidCell() const;
	bool IsFlankCell() const;
	bool IsRestrictedCell() const;
	FVector ToWorldLocation(const FVector& GridOrigin, float CellSize, float AirLayerHeight) const;
	FRotator GetFlankRotation() const;
	static bool IsValidCell(int32 Row, int32 Col);
	static bool IsFlankCell(int32 Row, int32 Col);
	static bool IsRestrictedCell(int32 Row, int32 Col);
	static FVector CellToWorldLocation(int32 Row, int32 Col, ETacGridLayer Layer, const FVector& GridOrigin, float CellSize, float AirLayerHeight);
	static FTacCoordinates WorldLocationToCell(const FVector& WorldLocation, const FVector& GridOrigin, float CellSize, float AirLayerHeight);
	static FRotator GetFlankRotation(int32 Row, int32 Col);
};

inline uint32 GetTypeHash(const FTacCoordinates& Coords)
{
	uint32 Hash = HashCombine(GetTypeHash(Coords.Row), GetTypeHash(Coords.Col));
	Hash = HashCombine(Hash, GetTypeHash(static_cast<uint8>(Coords.Layer)));
	return Hash;
}

inline EUnitOrientation RotateOrientationRight(EUnitOrientation O)
{
	// Top->Right->Bottom->Left->Top
	constexpr EUnitOrientation Table[] = {
		EUnitOrientation::GridRight,  // Top    -> Right
		EUnitOrientation::GridLeft,   // Bottom -> Left
		EUnitOrientation::GridTop,    // Left   -> Top
		EUnitOrientation::GridBottom  // Right  -> Bottom
	};
	return Table[static_cast<uint8>(O)];
}

inline EUnitOrientation RotateOrientationLeft(EUnitOrientation O)
{
	constexpr EUnitOrientation Table[] = {
		EUnitOrientation::GridLeft,   // Top    -> Left
		EUnitOrientation::GridRight,  // Bottom -> Right
		EUnitOrientation::GridBottom, // Left   -> Bottom
		EUnitOrientation::GridTop     // Right  -> Top
	};
	return Table[static_cast<uint8>(O)];
}

inline EUnitOrientation RotateOrientation(EUnitOrientation O, ERelativeTurn Turn)
{
	return Turn == ERelativeTurn::Right ? RotateOrientationRight(O) : RotateOrientationLeft(O);
}

inline EUnitOrientation OppositeOrientation(EUnitOrientation O)
{
	constexpr EUnitOrientation Opp[] = {
		EUnitOrientation::GridBottom, // Top    <-> Bottom
		EUnitOrientation::GridTop,
		EUnitOrientation::GridRight,  // Left   <-> Right
		EUnitOrientation::GridLeft
	};
	return Opp[static_cast<uint8>(O)];
}

// Returns the extra cell in the facing direction from Primary.
// Caller must validate with IsValidCell() before use.
inline FTacCoordinates GetExtraCellCoords(FTacCoordinates Primary, EUnitOrientation Orientation)
{
	switch (Orientation)
	{
	case EUnitOrientation::GridTop:    return FTacCoordinates(Primary.Row - 1, Primary.Col, Primary.Layer);
	case EUnitOrientation::GridBottom: return FTacCoordinates(Primary.Row + 1, Primary.Col, Primary.Layer);
	case EUnitOrientation::GridLeft:   return FTacCoordinates(Primary.Row, Primary.Col - 1, Primary.Layer);
	case EUnitOrientation::GridRight:  return FTacCoordinates(Primary.Row, Primary.Col + 1, Primary.Layer);
	}
	return FTacCoordinates::Invalid();
}

USTRUCT()
struct KBS_API FGridConstants
{
	GENERATED_BODY()
	static constexpr int32 GridSize = 5;
	static constexpr int32 TotalCells = 25;
	static constexpr int32 CenterRow = 2;
	static constexpr int32 ExcludedColLeft = 0;
	static constexpr int32 ExcludedColRight = 4;

	static inline const TArray<FIntPoint> OrthogonalOffsets = {
		FIntPoint(0, -1), FIntPoint(0, 1), FIntPoint(-1, 0), FIntPoint(1, 0)
	};

	static inline const TArray<FIntPoint> AllAdjacentOffsets = {
		FIntPoint(0, -1), FIntPoint(0, 1), FIntPoint(-1, 0), FIntPoint(1, 0),
		FIntPoint(-1, -1), FIntPoint(-1, 1), FIntPoint(1, -1), FIntPoint(1, 1)
	};
};