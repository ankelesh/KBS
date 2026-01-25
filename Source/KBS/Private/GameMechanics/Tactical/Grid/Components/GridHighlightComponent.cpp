#include "GameMechanics/Tactical/Grid/Components/GridHighlightComponent.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "Components/DecalComponent.h"
#include "GameplayTypes/GridCoordinates.h"
#include "GameplayTypes/DamageTypes.h"
UGridHighlightComponent::UGridHighlightComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UGridHighlightComponent::Initialize( USceneComponent* InRoot,
	UMaterialInterface* InMoveDecalMaterial, UMaterialInterface* InEnemyDecalMaterial)
{
	Root = InRoot;
	MoveDecalMaterial = InMoveDecalMaterial;
	EnemyDecalMaterial = InEnemyDecalMaterial;
}
void UGridHighlightComponent::CreateDecalPool()
{
	if (!Root || !MoveDecalMaterial || !EnemyDecalMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("GridHighlightComponent: Cannot create decal pool, missing references!"));
		return;
	}
	ATacBattleGrid* Grid = Cast<ATacBattleGrid>(GetOwner());
	if (!Grid || !Grid->Config)
	{
		UE_LOG(LogTemp, Error, TEXT("GridHighlightComponent: Cannot create decal pool, Grid or Config is null!"));
		return;
	}

	const float DecalHalfSize = Grid->Config->CellSize * 0.5f;
	for (int32 i = 0; i < FGridConstants::TotalCells; ++i)
	{
		UDecalComponent* MoveDecal = NewObject<UDecalComponent>(Grid);
		MoveDecal->RegisterComponent();
		MoveDecal->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
		MoveDecal->SetDecalMaterial(MoveDecalMaterial);
		MoveDecal->DecalSize = FVector(10.0f, DecalHalfSize, DecalHalfSize);
		MoveDecal->SetVisibility(false);
		MoveAllowedDecals.Add(MoveDecal);
		UDecalComponent* EnemyDecal = NewObject<UDecalComponent>(Grid);
		EnemyDecal->RegisterComponent();
		EnemyDecal->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
		EnemyDecal->SetDecalMaterial(EnemyDecalMaterial);
		EnemyDecal->DecalSize = FVector(10.0f, DecalHalfSize, DecalHalfSize);
		EnemyDecal->SetVisibility(false);
		EnemyDecals.Add(EnemyDecal);
	}
}
void UGridHighlightComponent::ShowValidMoves(const TArray<FTacCoordinates>& ValidCells)
{
	ATacBattleGrid* Grid = Cast<ATacBattleGrid>(GetOwner());
	if (!Grid || !Grid->GetDataManager())
	{
		return;
	}
	for (int32 i = 0; i < MoveAllowedDecals.Num(); ++i)
	{
		MoveAllowedDecals[i]->SetVisibility(false);
	}
	for (int32 i = 0; i < ValidCells.Num() && i < MoveAllowedDecals.Num(); ++i)
	{
		const FTacCoordinates& Coords = ValidCells[i];
		const FVector CellLocation = Grid->GetDataManager()->GetCellWorldLocation(Coords);
		MoveAllowedDecals[i]->SetWorldLocation(CellLocation);
		MoveAllowedDecals[i]->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));
		MoveAllowedDecals[i]->SetVisibility(true);
	}
}
void UGridHighlightComponent::ShowValidTargets(const TArray<FTacCoordinates>& TargetCells)
{
	ATacBattleGrid* Grid = Cast<ATacBattleGrid>(GetOwner());
	if (!Grid || !Grid->GetDataManager())
	{
		return;
	}
	for (int32 i = 0; i < EnemyDecals.Num(); ++i)
	{
		EnemyDecals[i]->SetVisibility(false);
	}
	for (int32 i = 0; i < TargetCells.Num() && i < EnemyDecals.Num(); ++i)
	{
		const FTacCoordinates& Coords = TargetCells[i];
		const FVector CellLocation = Grid->GetDataManager()->GetCellWorldLocation(Coords);
		EnemyDecals[i]->SetWorldLocation(CellLocation);
		EnemyDecals[i]->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));
		EnemyDecals[i]->SetVisibility(true);
	}
}
void UGridHighlightComponent::ShowHighlightsForTargeting(const TArray<FTacCoordinates>& Cells, ETargetReach TargetType)
{
	if (TargetType == ETargetReach::Movement)
	{
		ShowValidMoves(Cells);
	}
	else
	{
		ShowValidTargets(Cells);
	}
}
void UGridHighlightComponent::ClearHighlights()
{
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
