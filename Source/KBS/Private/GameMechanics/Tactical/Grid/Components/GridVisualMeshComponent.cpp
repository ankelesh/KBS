#include "GameMechanics/Tactical/Grid/Components/GridVisualMeshComponent.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "Components/BoxComponent.h"
#include "ProceduralMeshComponent.h"
#include "GameplayTypes/GridCoordinates.h"

UGridVisualMeshComponent::UGridVisualMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGridVisualMeshComponent::Initialize(ATacBattleGrid* InGrid)
{
	checkf(InGrid, TEXT("UGridVisualMeshComponent::Initialize called with null InGrid"));
	Grid = InGrid;
	BuildCellBoxes();
	BuildGroundMesh();
	BuildAirMesh();
}

void UGridVisualMeshComponent::BuildCellBoxes()
{
	const float CellSize = Grid->GetCellSize();
	const float AirLayerHeight = Grid->GetAirLayerHeight();
	AActor* Owner = GetOwner();
	USceneComponent* Root = Owner->GetRootComponent();

	constexpr int32 GridSize = FGridConstants::GridSize;
	const int32 TotalCells = GridSize * GridSize;
	GroundCellBoxes.Reserve(TotalCells);
	AirCellBoxes.Reserve(TotalCells);

	const float GroundBoxZ = Grid->GetGroundCellBoxZExtent();
	const float AirBoxZ = Grid->GetAirCellBoxZExtent();
	const FVector GroundExtent(CellSize * 0.5f, CellSize * 0.5f, GroundBoxZ);
	const FVector AirExtent(CellSize * 0.5f, CellSize * 0.5f, AirBoxZ);

	for (int32 Row = 0; Row < GridSize; ++Row)
	{
		for (int32 Col = 0; Col < GridSize; ++Col)
		{
			const float CenterX = Col * CellSize + CellSize * 0.5f;
			const float CenterY = Row * CellSize + CellSize * 0.5f;

			const FVector GroundCenter(CenterX, CenterY, 0.0f);
			const FVector AirCenter(CenterX, CenterY, AirLayerHeight + AirBoxZ);

			UBoxComponent* GroundBox = NewObject<UBoxComponent>(Owner, FName(*FString::Printf(TEXT("GroundCell_%d_%d"), Row, Col)));
			GroundBox->RegisterComponent();
			GroundBox->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
			GroundBox->SetRelativeLocation(GroundCenter);
			GroundBox->SetBoxExtent(GroundExtent);
			GroundBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			GroundBox->SetCollisionResponseToAllChannels(ECR_Ignore);
			GroundBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
			if (bDebugDrawBoxes) { GroundBox->SetHiddenInGame(false); GroundBox->ShapeColor = FColor::Green; }
			GroundCellBoxes.Add(GroundBox);

			UBoxComponent* AirBox = NewObject<UBoxComponent>(Owner, FName(*FString::Printf(TEXT("AirCell_%d_%d"), Row, Col)));
			AirBox->RegisterComponent();
			AirBox->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
			AirBox->SetRelativeLocation(AirCenter);
			AirBox->SetBoxExtent(AirExtent);
			AirBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			AirBox->SetCollisionResponseToAllChannels(ECR_Ignore);
			AirBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
			if (bDebugDrawBoxes) { AirBox->SetHiddenInGame(false); AirBox->ShapeColor = FColor::Cyan; }
			AirCellBoxes.Add(AirBox);
		}
	}
}

void UGridVisualMeshComponent::BuildGroundMesh()
{
	GroundMesh = NewObject<UProceduralMeshComponent>(GetOwner(), TEXT("GroundMesh"));
	GroundMesh->RegisterComponent();
	GroundMesh->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	GroundMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GroundMesh->SetTranslucentSortPriority(-10);

	const float CellSize = Grid->GetCellSize();
	const float Z = Grid->GetMeshZOffset();
	const UGridConfig* Config = Grid->Config;
	constexpr int32 GridSize = FGridConstants::GridSize;

	const TArray<FVector> Normals = { FVector(0,0,1), FVector(0,0,1), FVector(0,0,1), FVector(0,0,1) };
	const TArray<FLinearColor> Colors;
	const TArray<FProcMeshTangent> Tangents;

	for (int32 Row = 0; Row < GridSize; ++Row)
	{
		for (int32 Col = 0; Col < GridSize; ++Col)
		{
			const int32 SectionIndex = Row * GridSize + Col;

			const FVector V0(Col * CellSize,           Row * CellSize,       Z);
			const FVector V1((Col + 1) * CellSize,     Row * CellSize,       Z);
			const FVector V2((Col + 1) * CellSize, (Row + 1) * CellSize,     Z);
			const FVector V3(Col * CellSize,       (Row + 1) * CellSize,     Z);

			const TArray<FVector2D> UVs = { FVector2D(0,0), FVector2D(1,0), FVector2D(1,1), FVector2D(0,1) };

			GroundMesh->CreateMeshSection_LinearColor(SectionIndex, { V0, V1, V2, V3 }, { 0, 2, 1, 0, 3, 2 }, Normals, UVs, Colors, Tangents, false);

			if (FTacCoordinates::IsRestrictedCell(Row, Col))
			{
				GroundMesh->SetMeshSectionVisible(SectionIndex, false);
			}
			else
			{
				UMaterialInterface* Mat = FTacCoordinates::IsFlankCell(Row, Col)
					? Config->FlankCellMaterial
					: Config->NormalCellMaterial;
				GroundMesh->SetMaterial(SectionIndex, Mat);
			}
		}
	}
}

void UGridVisualMeshComponent::BuildAirMesh()
{
	AirMesh = NewObject<UProceduralMeshComponent>(GetOwner(), TEXT("AirMesh"));
	AirMesh->RegisterComponent();
	AirMesh->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	AirMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AirMesh->SetTranslucentSortPriority(-10);

	const float CellSize = Grid->GetCellSize();
	const float Z = Grid->GetAirLayerHeight() + Grid->GetMeshZOffset();
	UMaterialInterface* Mat = Grid->Config->AirCellMaterial;
	constexpr int32 GridSize = FGridConstants::GridSize;

	const TArray<FVector> Normals = { FVector(0,0,1), FVector(0,0,1), FVector(0,0,1), FVector(0,0,1) };
	const TArray<FLinearColor> Colors;
	const TArray<FProcMeshTangent> Tangents;

	for (int32 Row = 0; Row < GridSize; ++Row)
	{
		for (int32 Col = 0; Col < GridSize; ++Col)
		{
			const int32 SectionIndex = Row * GridSize + Col;

			const FVector V0(Col * CellSize,           Row * CellSize,       Z);
			const FVector V1((Col + 1) * CellSize,     Row * CellSize,       Z);
			const FVector V2((Col + 1) * CellSize, (Row + 1) * CellSize,     Z);
			const FVector V3(Col * CellSize,       (Row + 1) * CellSize,     Z);

			const TArray<FVector2D> UVs = { FVector2D(0,0), FVector2D(1,0), FVector2D(1,1), FVector2D(0,1) };

			AirMesh->CreateMeshSection_LinearColor(SectionIndex, { V0, V1, V2, V3 }, { 0, 2, 1, 0, 3, 2 }, Normals, UVs, Colors, Tangents, false);
			AirMesh->SetMaterial(SectionIndex, Mat);
		}
	}
}
