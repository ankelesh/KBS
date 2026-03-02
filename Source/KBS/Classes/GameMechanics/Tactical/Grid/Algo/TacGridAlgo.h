#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameMechanics/Units/Unit.h"

namespace KbsAlgo
{
	enum class EAdjacencyMode : uint8 { AllDirections, OrthogonalOnly };
	namespace Detail
	{
		template <template<typename...> class TContainer>
		struct TCollectionPolicy;

		template <>
		struct TCollectionPolicy<TSet>
		{
			static TSet<FTacCoordinates> Convert(TSet<FTacCoordinates>&& InSet)
			{
				return MoveTemp(InSet);
			}
		};

		template <>
		struct TCollectionPolicy<TArray>
		{
			static TArray<FTacCoordinates> Convert(TSet<FTacCoordinates>&& InSet)
			{
				return InSet.Array();
			}
		};
		


		template <typename D>
		bool ForEachAdjacentFrom(FTacCoordinates Origin, D DataManager,
		                         TFunctionRef<bool(AUnit*, const FTacCoordinates&)> Visitor,
		                         bool bIsTakingFlank, const TArray<FIntPoint>& Offsets)
		{
			for (const FIntPoint& Offset : Offsets)
			{
				FTacCoordinates Coords(Origin.Row + Offset.X, Origin.Col + Offset.Y, Origin.Layer);
				if (!Coords.IsValidCell()) continue;
				if (!bIsTakingFlank && Coords.IsFlankCell()) continue;
				if (Visitor(DataManager->GetUnit(Coords), Coords))
					return true;
			}
			return false;
		}

		template <typename D>
		bool ForEachInArea(D DataManager, FTacCoordinates CenterCell, const FAreaShape& AreaShape,
		                   TFunctionRef<bool(AUnit*, const FTacCoordinates&)> Visitor)
		{
			auto CheckLayer = [&](ETacGridLayer Layer) -> bool
			{
				for (const FIntPoint& RelativeOffset : AreaShape.RelativeCells)
				{
					FTacCoordinates Coords = CenterCell + RelativeOffset;
					Coords.Layer = Layer;
					if (Visitor(DataManager->GetUnit(Coords), Coords))
						return true;
				}
				return false;
			};
			switch (AreaShape.ShapeLayering)
			{
			case EShapeLayering::AirArea: return CheckLayer(ETacGridLayer::Air);
			case EShapeLayering::GroundArea: return CheckLayer(ETacGridLayer::Ground);
			case EShapeLayering::BothLayerArea: return CheckLayer(ETacGridLayer::Ground) || CheckLayer(
					ETacGridLayer::Air);
			}
			return false;
		}
	} // namespace Detail

	template <template<typename...> class TContainer, typename D>
	TContainer<AUnit*> ExtractUnits(const TContainer<FTacCoordinates>& Source, D DataManager)
	{
		TSet<AUnit*> Units;
		for (int i = 0; i < Source.Num(); i++)
			if (AUnit* Unit = DataManager->GetUnit(Source[i]))
				Units.Add(Unit);
		TContainer<AUnit*> Deduplicated;
		Algo::Copy(Units, Deduplicated);
		return Deduplicated;
	}
	
	template <template<typename...> class TContainer, typename D>
	TContainer<FTacCoordinates> CollectAdjacentCellsIf(AUnit* SourceUnit, D DataManager,
	                                                   TFunctionRef<bool(
		                                                   const AUnit*, const AUnit*,
		                                                   const FTacCoordinates&)> Predicate,
	                                                   bool bIsTakingFlank = false,
	                                                   EAdjacencyMode Mode = EAdjacencyMode::AllDirections)
	{
		TSet<FTacCoordinates> FoundCells;
		const FUnitGridMetadata& Metadata = SourceUnit->GetGridMetadata();
		if (!Metadata.IsValid())
			return Detail::TCollectionPolicy<TContainer>::Convert(MoveTemp(FoundCells));

		const bool bIsMultiCell = Metadata.HasExtraCell();
		const TArray<FIntPoint>& Offsets = (Mode == EAdjacencyMode::OrthogonalOnly)
			? FGridConstants::OrthogonalOffsets : FGridConstants::AllAdjacentOffsets;

		auto Visit = [&](AUnit* Occupant, const FTacCoordinates& Coords) -> bool
		{
			if (bIsMultiCell && (Coords == Metadata.Coords || Coords == Metadata.ExtraCell))
				return false;
			if (Predicate(SourceUnit, Occupant, Coords)) FoundCells.Add(Coords);
			return false;
		};

		Detail::ForEachAdjacentFrom(Metadata.Coords, DataManager, Visit, bIsTakingFlank, Offsets);
		if (bIsMultiCell)
			Detail::ForEachAdjacentFrom(Metadata.ExtraCell, DataManager, Visit, bIsTakingFlank, Offsets);
		return Detail::TCollectionPolicy<TContainer>::Convert(MoveTemp(FoundCells));
	}

	template <typename D>
	bool AnyAdjacentCellOf(AUnit* SourceUnit, D DataManager,
	                       TFunctionRef<bool(const AUnit*, const AUnit*, const FTacCoordinates&)> Predicate,
	                       bool bIsTakingFlank = false,
	                       EAdjacencyMode Mode = EAdjacencyMode::AllDirections)
	{
		const FUnitGridMetadata& Metadata = SourceUnit->GetGridMetadata();
		if (!Metadata.IsValid())
			return false;

		const bool bIsMultiCell = Metadata.HasExtraCell();
		const TArray<FIntPoint>& Offsets = (Mode == EAdjacencyMode::OrthogonalOnly)
			? FGridConstants::OrthogonalOffsets : FGridConstants::AllAdjacentOffsets;

		auto Visit = [&](AUnit* Occupant, const FTacCoordinates& Coords) -> bool
		{
			if (bIsMultiCell && (Coords == Metadata.Coords || Coords == Metadata.ExtraCell))
				return false;
			return Predicate(SourceUnit, Occupant, Coords);
		};

		if (Detail::ForEachAdjacentFrom(Metadata.Coords, DataManager, Visit, bIsTakingFlank, Offsets))
			return true;
		if (bIsMultiCell)
			return Detail::ForEachAdjacentFrom(Metadata.ExtraCell, DataManager, Visit, bIsTakingFlank, Offsets);
		return false;
	}


	template <typename D>
	bool AdjacentContains(AUnit* SourceUnit, D DataManager, FTacCoordinates TargetCoord,
	                      TFunctionRef<bool(const AUnit*, const AUnit*, const FTacCoordinates&)> Predicate,
	                      bool bIsTakingFlank = false,
	                      EAdjacencyMode Mode = EAdjacencyMode::AllDirections)
	{
		if (!TargetCoord.IsValidCell()) return false;
		if (!bIsTakingFlank && TargetCoord.IsFlankCell()) return false;

		const FUnitGridMetadata& Metadata = SourceUnit->GetGridMetadata();
		if (!Metadata.IsValid()) return false;

		const TArray<FIntPoint>& Offsets = (Mode == EAdjacencyMode::OrthogonalOnly)
			? FGridConstants::OrthogonalOffsets : FGridConstants::AllAdjacentOffsets;

		auto IsAdjacentTo = [&](FTacCoordinates Origin) -> bool
		{
			return TargetCoord.Layer == Origin.Layer
				&& Offsets.Contains(FIntPoint(TargetCoord.Row - Origin.Row, TargetCoord.Col - Origin.Col));
		};

		if (!IsAdjacentTo(Metadata.Coords) && (!Metadata.HasExtraCell() || !IsAdjacentTo(Metadata.ExtraCell)))
			return false;

		return Predicate(SourceUnit, DataManager->GetUnit(TargetCoord), TargetCoord);
	}


	template <template<typename...> class TContainer, typename D>
	TContainer<FTacCoordinates> CollectCellsInArea(AUnit* SourceUnit, D DataManager,
	                                               TFunctionRef<bool
		                                               (const AUnit*, const AUnit*, const FTacCoordinates&)> Predicate,
	                                               FTacCoordinates CenterCell, const FAreaShape& AreaShape)
	{
		TSet<FTacCoordinates> FoundCells;
		Detail::ForEachInArea(DataManager, CenterCell, AreaShape,
		                      [&](AUnit* Occupant, const FTacCoordinates& Coords) -> bool
		                      {
			                      if (Predicate(SourceUnit, Occupant, Coords)) FoundCells.Add(Coords);
			                      return false;
		                      });
		return Detail::TCollectionPolicy<TContainer>::Convert(MoveTemp(FoundCells));
	}

	template <typename D>
	bool AnyUnitInAreaOf(AUnit* SourceUnit, D DataManager,
	                   TFunctionRef<bool(const AUnit*, const AUnit*, const FTacCoordinates&)> Predicate,
	                   FTacCoordinates CenterCell, const FAreaShape& AreaShape)
	{
		return Detail::ForEachInArea(DataManager, CenterCell, AreaShape,
		                             [&](AUnit* Occupant, const FTacCoordinates& Coords) -> bool
		                             {
			                             return Predicate(SourceUnit, Occupant, Coords);
		                             });
	}


	template <typename D>
	bool AreaContains(AUnit* SourceUnit, D DataManager, FTacCoordinates TargetCoord,
	                  TFunctionRef<bool(const AUnit*, const AUnit*, const FTacCoordinates&)> Predicate,
	                  FTacCoordinates CenterCell, const FAreaShape& AreaShape)
	{
		auto IsInLayer = [&](ETacGridLayer Layer) -> bool
		{
			if (TargetCoord.Layer != Layer) return false;
			for (const FIntPoint& RelOffset : AreaShape.RelativeCells)
			{
				FTacCoordinates Candidate = CenterCell + RelOffset;
				Candidate.Layer = Layer;
				if (Candidate == TargetCoord) return true;
			}
			return false;
		};

		bool bInArea = false;
		switch (AreaShape.ShapeLayering)
		{
		case EShapeLayering::AirArea:       bInArea = IsInLayer(ETacGridLayer::Air); break;
		case EShapeLayering::GroundArea:    bInArea = IsInLayer(ETacGridLayer::Ground); break;
		case EShapeLayering::BothLayerArea: bInArea = IsInLayer(ETacGridLayer::Ground) || IsInLayer(ETacGridLayer::Air); break;
		}
		if (!bInArea) return false;
		return Predicate(SourceUnit, DataManager->GetUnit(TargetCoord), TargetCoord);
	}


} // namespace KbsAlgo
