#pragma once
#include "CoreMinimal.h"
#include "GridCoordinates.generated.h"

namespace Tactical {
	enum class EBattleLayer : uint8;


	USTRUCT(BlueprintType)
		struct FTacCoordinates
	{
		GENERATED_BODY()
		int32 Row;
		int32 Col;
		EBattleLayer Layer;

		int32 X() const { return Row; }
		int32 Y() const { return Col; }
		EBattleLayer Z() const { return Layer; }
		FTacCoordinates() : Row(0), Col(0), Layer(EBattleLayer::Ground) {}
		FTacCoordinates(int32 Row, int32 Col) : Row(Row), Col(Col), Layer(EBattleLayer::Ground) {}
		FTacCoordinates(int32 Row, int32 Col, EBattleLayer Layer) : Row(Row), Col(Col), Layer(Layer) {}

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
		static FVector CellToWorldLocation(int32 Row, int32 Col, EBattleLayer Layer, const FVector& GridOrigin, float CellSize, float AirLayerHeight);
		static FTacCoordinates WorldLocationToCell(const FVector& WorldLocation, const FVector& GridOrigin, float CellSize);
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
}; // namespace Tactical