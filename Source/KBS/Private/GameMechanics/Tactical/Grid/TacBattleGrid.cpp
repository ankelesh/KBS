#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Units/Weapons/Weapon.h"
#include "Components/DecalComponent.h"

ATacBattleGrid::ATacBattleGrid()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Root->SetMobility(EComponentMobility::Movable);

	GroundLayer.Init(nullptr, TotalCells);
	AirLayer.Init(nullptr, TotalCells);

	AttackerTeam = CreateDefaultSubobject<UBattleTeam>(TEXT("AttackerTeam"));
	AttackerTeam->SetTeamSide(ETeamSide::Attacker);

	DefenderTeam = CreateDefaultSubobject<UBattleTeam>(TEXT("DefenderTeam"));
	DefenderTeam->SetTeamSide(ETeamSide::Defender);
}

void ATacBattleGrid::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	FlushPersistentDebugLines(GetWorld());

	for (int32 Row = 0; Row < GridSize; ++Row)
	{
		for (int32 Col = 0; Col < GridSize; ++Col)
		{
			if (IsRestrictedCell(Row, Col))
			{
				continue;
			}

			const bool bIsFlank = IsFlankCell(Row, Col);

			FVector CellCenter = GetCellWorldLocation(Row, Col, EBattleLayer::Ground);
			FColor GroundColor = bIsFlank ? FColor(138, 43, 226) : FColor::Green;
			DrawDebugBox(GetWorld(), CellCenter, FVector(CellSize * 0.5f, CellSize * 0.5f, 5.0f),
				GroundColor, true, -1.0f, 0, 2.0f);

			CellCenter = GetCellWorldLocation(Row, Col, EBattleLayer::Air);
			DrawDebugBox(GetWorld(), CellCenter, FVector(CellSize * 0.5f, CellSize * 0.5f, 5.0f),
				FColor::Cyan, true, -1.0f, 0, 1.0f);
		}
	}
#endif

	// Position units assigned in editor
	for (int32 i = 0; i < GroundLayer.Num(); ++i)
	{
		if (GroundLayer[i])
		{
			const int32 Row = i / GridSize;
			const int32 Col = i % GridSize;
			if (IsValidCell(Row, Col, EBattleLayer::Ground))
			{
				GroundLayer[i]->SetActorLocation(GetCellWorldLocation(Row, Col, EBattleLayer::Ground));
				GroundLayer[i]->SetGridPosition(Row, Col, EBattleLayer::Ground);
			}
			else
			{
				GroundLayer[i] = nullptr;
			}
		}
	}

	for (int32 i = 0; i < AirLayer.Num(); ++i)
	{
		if (AirLayer[i])
		{
			const int32 Row = i / GridSize;
			const int32 Col = i % GridSize;
			if (IsValidCell(Row, Col, EBattleLayer::Air))
			{
				AirLayer[i]->SetActorLocation(GetCellWorldLocation(Row, Col, EBattleLayer::Air));
				AirLayer[i]->SetGridPosition(Row, Col, EBattleLayer::Air);
			}
			else
			{
				AirLayer[i] = nullptr;
			}
		}
	}
}

void ATacBattleGrid::BeginPlay()
{
	Super::BeginPlay();

	for (int32 i = 0; i < TotalCells; ++i)
	{
		UDecalComponent* MoveDecal = NewObject<UDecalComponent>(this);
		MoveDecal->RegisterComponent();
		MoveDecal->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
		MoveDecal->SetDecalMaterial(MoveAllowedDecalMaterial);
		MoveDecal->DecalSize = FVector(10.0f, CellSize * 0.5f, CellSize * 0.5f);
		MoveDecal->SetVisibility(false);
		MoveAllowedDecals.Add(MoveDecal);

		UDecalComponent* EnemyDecal = NewObject<UDecalComponent>(this);
		EnemyDecal->RegisterComponent();
		EnemyDecal->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
		EnemyDecal->SetDecalMaterial(EnemyDecalMaterial);
		EnemyDecal->DecalSize = FVector(10.0f, CellSize * 0.5f, CellSize * 0.5f);
		EnemyDecal->SetVisibility(false);
		EnemyDecals.Add(EnemyDecal);
	}
}

bool ATacBattleGrid::IsValidCell(int32 Row, int32 Col, EBattleLayer Layer) const
{
	if (Row < 0 || Row >= GridSize || Col < 0 || Col >= GridSize)
	{
		return false;
	}

	if (Row == CenterRow && (Col == ExcludedColLeft || Col == ExcludedColRight))
	{
		return false;
	}

	return true;
}

bool ATacBattleGrid::PlaceUnit(AUnit* Unit, int32 Row, int32 Col, EBattleLayer Layer)
{
	if (!Unit || !IsValidCell(Row, Col, Layer))
	{
		return false;
	}

	const int32 Index = CoordsToIndex(Row, Col);
	TArray<TObjectPtr<AUnit>>& LayerArray = GetLayer(Layer);

	if (LayerArray[Index] != nullptr)
	{
		return false;
	}

	LayerArray[Index] = Unit;
	Unit->SetGridPosition(Row, Col, Layer);
	Unit->SetActorLocation(GetCellWorldLocation(Row, Col, Layer));

	return true;
}

AUnit* ATacBattleGrid::GetUnit(int32 Row, int32 Col, EBattleLayer Layer) const
{
	if (!IsValidCell(Row, Col, Layer))
	{
		return nullptr;
	}

	const int32 Index = CoordsToIndex(Row, Col);
	const TArray<TObjectPtr<AUnit>>& LayerArray = GetLayer(Layer);

	return LayerArray[Index];
}

bool ATacBattleGrid::RemoveUnit(int32 Row, int32 Col, EBattleLayer Layer)
{
	if (!IsValidCell(Row, Col, Layer))
	{
		return false;
	}

	const int32 Index = CoordsToIndex(Row, Col);
	TArray<TObjectPtr<AUnit>>& LayerArray = GetLayer(Layer);

	if (LayerArray[Index] == nullptr)
	{
		return false;
	}

	LayerArray[Index] = nullptr;
	return true;
}

int32 ATacBattleGrid::CoordsToIndex(int32 Row, int32 Col) const
{
	return Row + Col * GridSize;
}

TArray<TObjectPtr<AUnit>>& ATacBattleGrid::GetLayer(EBattleLayer Layer)
{
	return Layer == EBattleLayer::Ground ? GroundLayer : AirLayer;
}

const TArray<TObjectPtr<AUnit>>& ATacBattleGrid::GetLayer(EBattleLayer Layer) const
{
	return Layer == EBattleLayer::Ground ? GroundLayer : AirLayer;
}

FVector ATacBattleGrid::GetCellWorldLocation(int32 Row, int32 Col, EBattleLayer Layer) const
{
	const FVector GridOrigin = GetActorLocation();
	const float X = Col * CellSize + CellSize * 0.5f;
	const float Y = Row * CellSize + CellSize * 0.5f;
	const float Z = (Layer == EBattleLayer::Air) ? AirLayerHeight : 0.0f;

	return GridOrigin + FVector(X, Y, Z);
}

bool ATacBattleGrid::IsFlankCell(int32 Row, int32 Col) const
{
	if (Row == CenterRow)
	{
		return false;
	}

	return Col == ExcludedColLeft || Col == ExcludedColRight;
}

bool ATacBattleGrid::IsRestrictedCell(int32 Row, int32 Col) const
{
	return Row == CenterRow && (Col == ExcludedColLeft || Col == ExcludedColRight);
}

UBattleTeam* ATacBattleGrid::GetTeamForUnit(AUnit* Unit) const
{
	if (!Unit)
	{
		return nullptr;
	}

	if (AttackerTeam && AttackerTeam->ContainsUnit(Unit))
	{
		return AttackerTeam;
	}

	if (DefenderTeam && DefenderTeam->ContainsUnit(Unit))
	{
		return DefenderTeam;
	}

	return nullptr;
}

UBattleTeam* ATacBattleGrid::GetEnemyTeam(AUnit* Unit) const
{
	UBattleTeam* UnitTeam = GetTeamForUnit(Unit);

	if (UnitTeam == AttackerTeam)
	{
		return DefenderTeam;
	}

	if (UnitTeam == DefenderTeam)
	{
		return AttackerTeam;
	}

	return nullptr;
}

TArray<FIntPoint> ATacBattleGrid::GetValidMoveCells(AUnit* Unit) const
{
	TArray<FIntPoint> ValidCells;

	if (!Unit)
	{
		return ValidCells;
	}

	const int32 UnitRow = Unit->GetGridRow();
	const int32 UnitCol = Unit->GetGridCol();
	const EBattleLayer UnitLayer = Unit->GetGridLayer();
	UBattleTeam* UnitTeam = GetTeamForUnit(Unit);
	UBattleTeam* EnemyTeam = GetEnemyTeam(Unit);

	if (UnitLayer == EBattleLayer::Air)
	{
		for (int32 Row = 0; Row < GridSize; ++Row)
		{
			for (int32 Col = 0; Col < GridSize; ++Col)
			{
				if (IsRestrictedCell(Row, Col))
				{
					continue;
				}

				AUnit* OccupyingUnit = GetUnit(Row, Col, UnitLayer);
				if (!OccupyingUnit || (UnitTeam && UnitTeam->ContainsUnit(OccupyingUnit)))
				{
					ValidCells.Add(FIntPoint(Col, Row));
				}
			}
		}
	}
	else
	{
		const TArray<FIntPoint> AdjacentOffsets = {
			FIntPoint(0, -1), FIntPoint(0, 1), FIntPoint(-1, 0), FIntPoint(1, 0)
		};

		for (const FIntPoint& Offset : AdjacentOffsets)
		{
			const int32 TargetRow = UnitRow + Offset.Y;
			const int32 TargetCol = UnitCol + Offset.X;

			if (!IsValidCell(TargetRow, TargetCol, UnitLayer))
			{
				continue;
			}

			if (IsFlankCell(TargetRow, TargetCol))
			{
				if (UnitTeam == AttackerTeam && TargetRow <= 1)
				{
					continue;
				}

				if (UnitTeam == DefenderTeam && TargetRow >= 3)
				{
					continue;
				}
			}

			AUnit* OccupyingUnit = GetUnit(TargetRow, TargetCol, UnitLayer);
			if (!OccupyingUnit || (UnitTeam && UnitTeam->ContainsUnit(OccupyingUnit)))
			{
				ValidCells.Add(FIntPoint(TargetCol, TargetRow));
			}
		}
	}

	return ValidCells;
}

bool ATacBattleGrid::MoveUnit(AUnit* Unit, int32 TargetRow, int32 TargetCol)
{
	if (!Unit)
	{
		return false;
	}

	const TArray<FIntPoint> ValidCells = GetValidMoveCells(Unit);
	const FIntPoint TargetCell(TargetCol, TargetRow);

	if (!ValidCells.Contains(TargetCell))
	{
		return false;
	}

	const int32 CurrentRow = Unit->GetGridRow();
	const int32 CurrentCol = Unit->GetGridCol();
	const EBattleLayer Layer = Unit->GetGridLayer();

	AUnit* TargetOccupant = GetUnit(TargetRow, TargetCol, Layer);

	if (TargetOccupant)
	{
		RemoveUnit(TargetRow, TargetCol, Layer);
		RemoveUnit(CurrentRow, CurrentCol, Layer);

		PlaceUnit(Unit, TargetRow, TargetCol, Layer);
		PlaceUnit(TargetOccupant, CurrentRow, CurrentCol, Layer);
	}
	else
	{
		RemoveUnit(CurrentRow, CurrentCol, Layer);
		PlaceUnit(Unit, TargetRow, TargetCol, Layer);
	}

	return true;
}

TArray<FIntPoint> ATacBattleGrid::GetValidTargetCells(AUnit* Unit, ETargetReach Reach) const
{
	TArray<FIntPoint> TargetCells;

	if (!Unit)
	{
		return TargetCells;
	}

	const int32 UnitRow = Unit->GetGridRow();
	const int32 UnitCol = Unit->GetGridCol();
	const EBattleLayer UnitLayer = Unit->GetGridLayer();
	UBattleTeam* EnemyTeam = GetEnemyTeam(Unit);

	if (!EnemyTeam)
	{
		return TargetCells;
	}

	if (Reach == ETargetReach::ClosestEnemies)
	{
		int32 MinDistance = TNumericLimits<int32>::Max();

		for (AUnit* EnemyUnit : EnemyTeam->GetUnits())
		{
			if (!EnemyUnit)
			{
				continue;
			}

			const int32 Distance = FMath::Abs(EnemyUnit->GetGridRow() - UnitRow) + FMath::Abs(EnemyUnit->GetGridCol() - UnitCol);

			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				TargetCells.Empty();
				TargetCells.Add(FIntPoint(EnemyUnit->GetGridCol(), EnemyUnit->GetGridRow()));
			}
			else if (Distance == MinDistance)
			{
				TargetCells.Add(FIntPoint(EnemyUnit->GetGridCol(), EnemyUnit->GetGridRow()));
			}
		}
	}
	else if (Reach == ETargetReach::Flank)
	{
		const TArray<int32> AdjacentCols = { UnitCol - 1, UnitCol + 1 };

		for (int32 Col : AdjacentCols)
		{
			if (Col < 0 || Col >= GridSize)
			{
				continue;
			}

			for (int32 Row = 0; Row < GridSize; ++Row)
			{
				AUnit* TargetUnit = GetUnit(Row, Col, UnitLayer);

				if (TargetUnit && EnemyTeam->ContainsUnit(TargetUnit))
				{
					const int32 DistanceToCenter = FMath::Abs(Row - CenterRow);
					TargetCells.Add(FIntPoint(Col, Row));
				}
			}
		}

		TArray<FIntPoint> ClosestToCenterCells;
		int32 MinDistToCenter = TNumericLimits<int32>::Max();

		for (const FIntPoint& Cell : TargetCells)
		{
			const int32 Dist = FMath::Abs(Cell.Y - CenterRow);

			if (Dist < MinDistToCenter)
			{
				MinDistToCenter = Dist;
				ClosestToCenterCells.Empty();
				ClosestToCenterCells.Add(Cell);
			}
			else if (Dist == MinDistToCenter)
			{
				ClosestToCenterCells.Add(Cell);
			}
		}

		TargetCells = ClosestToCenterCells;
	}
	else if (Reach == ETargetReach::AnyEnemy)
	{
		for (AUnit* EnemyUnit : EnemyTeam->GetUnits())
		{
			if (EnemyUnit)
			{
				TargetCells.Add(FIntPoint(EnemyUnit->GetGridCol(), EnemyUnit->GetGridRow()));
			}
		}
	}

	return TargetCells;
}

TArray<AUnit*> ATacBattleGrid::GetValidTargetUnits(AUnit* Unit, ETargetReach Reach) const
{
	TArray<AUnit*> TargetUnits;

	const TArray<FIntPoint> TargetCells = GetValidTargetCells(Unit, Reach);

	for (const FIntPoint& Cell : TargetCells)
	{
		AUnit* TargetUnit = GetUnit(Cell.Y, Cell.X, Unit->GetGridLayer());

		if (TargetUnit)
		{
			TargetUnits.Add(TargetUnit);
		}
	}

	return TargetUnits;
}

void ATacBattleGrid::SelectUnit(AUnit* Unit)
{
	if (!Unit)
	{
		ClearSelection();
		return;
	}

	SelectedUnit = Unit;

	const TArray<FIntPoint> ValidCells = GetValidMoveCells(Unit);

	for (int32 i = 0; i < MoveAllowedDecals.Num(); ++i)
	{
		MoveAllowedDecals[i]->SetVisibility(false);
	}

	for (int32 i = 0; i < ValidCells.Num() && i < MoveAllowedDecals.Num(); ++i)
	{
		const FIntPoint& Cell = ValidCells[i];
		const FVector CellLocation = GetCellWorldLocation(Cell.Y, Cell.X, Unit->GetGridLayer());

		MoveAllowedDecals[i]->SetWorldLocation(CellLocation + FVector(0.0f, 0.0f, 10.0f));
		MoveAllowedDecals[i]->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));
		MoveAllowedDecals[i]->SetVisibility(true);
	}
}

bool ATacBattleGrid::TryMoveSelectedUnit(int32 TargetRow, int32 TargetCol)
{
	if (!SelectedUnit)
	{
		return false;
	}

	const bool bSuccess = MoveUnit(SelectedUnit, TargetRow, TargetCol);

	if (bSuccess)
	{
		ClearSelection();
	}

	return bSuccess;
}

void ATacBattleGrid::ClearSelection()
{
	SelectedUnit = nullptr;

	for (UDecalComponent* Decal : MoveAllowedDecals)
	{
		if (Decal)
		{
			Decal->SetVisibility(false);
		}
	}

	for (UDecalComponent* Decal : EnemyDecals)
	{
		if (Decal)
		{
			Decal->SetVisibility(false);
		}
	}
}

FIntPoint ATacBattleGrid::GetCellFromWorldLocation(FVector WorldLocation) const
{
	const FVector GridOrigin = GetActorLocation();
	const FVector LocalPos = WorldLocation - GridOrigin;

	const int32 Col = FMath::FloorToInt(LocalPos.X / CellSize);
	const int32 Row = FMath::FloorToInt(LocalPos.Y / CellSize);

	return FIntPoint(FMath::Clamp(Col, 0, GridSize - 1), FMath::Clamp(Row, 0, GridSize - 1));
}
