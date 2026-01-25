#pragma once
#include "CoreMinimal.h"
#include "GridCoordinates.generated.h"


UENUM(BlueprintType)
enum class ETacGridLayer : uint8
{
	Ground = 0 UMETA(DisplayName = "Ground"),
	Air = 1 UMETA(DisplayName = "Air")
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