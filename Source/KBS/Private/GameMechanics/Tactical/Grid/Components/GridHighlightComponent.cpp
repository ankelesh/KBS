#include "GameMechanics/Tactical/Grid/Components/GridHighlightComponent.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "Components/DecalComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "GameplayTypes/GridCoordinates.h"

UGridHighlightComponent::UGridHighlightComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGridHighlightComponent::Initialize(USceneComponent* InRoot, UGridConfig* InConfig)
{
	Root = InRoot;
	Config = InConfig;
}

void UGridHighlightComponent::CreateHighlightPool()
{
	if (!Root || !Config)
	{
		UE_LOG(LogTemp, Error, TEXT("GridHighlightComponent: Cannot create highlight pool, missing Root or Config!"));
		return;
	}

	ATacBattleGrid* Grid = Cast<ATacBattleGrid>(GetOwner());
	if (!Grid)
	{
		UE_LOG(LogTemp, Error, TEXT("GridHighlightComponent: Cannot create highlight pool, Grid is null!"));
		return;
	}

	const float DecalHalfSize = Config->CellSize * 0.5f;
	constexpr int32 PoolSize = FGridConstants::TotalCells * 2; // 50 total

	HighlightPool.Reserve(PoolSize);

	for (int32 i = 0; i < PoolSize; ++i)
	{
		FHighlightInstance Instance;

		// Create decal
		Instance.Decal = NewObject<UDecalComponent>(Grid);
		Instance.Decal->RegisterComponent();
		Instance.Decal->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
		Instance.Decal->DecalSize = FVector(10.0f, DecalHalfSize, DecalHalfSize);
		Instance.Decal->SetVisibility(false);

		// Create Niagara component (if systems are configured)
		Instance.Niagara = NewObject<UNiagaraComponent>(Grid);
		Instance.Niagara->RegisterComponent();
		Instance.Niagara->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
		Instance.Niagara->SetAutoActivate(false);
		Instance.Niagara->Deactivate();

		Instance.bInUse = false;

		HighlightPool.Add(Instance);
	}

	UE_LOG(LogTemp, Log, TEXT("GridHighlightComponent: Created pool of %d highlight instances"), PoolSize);
}

UMaterialInterface* UGridHighlightComponent::GetMaterialForType(EHighlightType Type) const
{
	if (!Config) return nullptr;

	switch (Type)
	{
	case EHighlightType::Movement:
		return Config->MovementDecalMaterial;
	case EHighlightType::Attack:
		return Config->AttackDecalMaterial;
	case EHighlightType::CurrentSelected:
		return Config->CurrentSelectedDecalMaterial;
	case EHighlightType::Friendly:
		return Config->FriendlyDecalMaterial;
	default:
		return nullptr;
	}
}

UNiagaraSystem* UGridHighlightComponent::GetNiagaraSystemForType(EHighlightType Type) const
{
	if (!Config) return nullptr;

	const int32 TypeIndex = static_cast<int32>(Type);
	if (Config->HighlightNiagaraSystems.IsValidIndex(TypeIndex))
	{
		return Config->HighlightNiagaraSystems[TypeIndex];
	}

	return nullptr;
}

TArray<FTacCoordinates> UGridHighlightComponent::DeduplicateCoordinates(
	const TArray<FTacCoordinates>& NewCells, EHighlightType NewType)
{
	TArray<FTacCoordinates> Deduplicated;
	Deduplicated.Reserve(NewCells.Num());

	for (const FTacCoordinates& Cell : NewCells)
	{
		bool bAlreadyHighlighted = false;

		// Check if this cell is already highlighted by ANY type
		for (const auto& Pair : ActiveHighlightIndices)
		{
			for (int32 PoolIndex : Pair.Value)
			{
				if (HighlightPool[PoolIndex].Coords == Cell)
				{
					bAlreadyHighlighted = true;
					break;
				}
			}
			if (bAlreadyHighlighted) break;
		}

		if (!bAlreadyHighlighted)
		{
			Deduplicated.Add(Cell);
		}
	}

	return Deduplicated;
}

void UGridHighlightComponent::ShowHighlights(const TArray<FTacCoordinates>& Cells, EHighlightType HighlightType)
{
	ATacBattleGrid* Grid = Cast<ATacBattleGrid>(GetOwner());
	if (!Grid || !Grid->GetDataManager())
	{
		return;
	}

	// Deduplicate against already active highlights
	TArray<FTacCoordinates> DeduplicatedCells = DeduplicateCoordinates(Cells, HighlightType);

	// Get material and Niagara system for this type
	UMaterialInterface* Material = GetMaterialForType(HighlightType);
	UNiagaraSystem* NiagaraSystem = GetNiagaraSystemForType(HighlightType);

	if (!Material)
	{
		UE_LOG(LogTemp, Warning, TEXT("GridHighlightComponent: No material configured for highlight type %d"),
			static_cast<int32>(HighlightType));
		return;
	}

	// Allocate instances from pool
	TArray<int32>& ActiveIndices = ActiveHighlightIndices.FindOrAdd(HighlightType);

	for (const FTacCoordinates& Coords : DeduplicatedCells)
	{
		// Find free instance in pool
		int32 FreeIndex = INDEX_NONE;
		for (int32 i = 0; i < HighlightPool.Num(); ++i)
		{
			if (!HighlightPool[i].bInUse)
			{
				FreeIndex = i;
				break;
			}
		}

		if (FreeIndex == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("GridHighlightComponent: Pool exhausted! Cannot show more highlights."));
			break;
		}

		// Configure and activate instance
		FHighlightInstance& Instance = HighlightPool[FreeIndex];
		Instance.bInUse = true;
		Instance.Type = HighlightType;
		Instance.Coords = Coords;

		const FVector CellLocation = Grid->GetDataManager()->GetCellWorldLocation(Coords);

		// Setup decal
		Instance.Decal->SetDecalMaterial(Material);
		Instance.Decal->SetWorldLocation(CellLocation);
		Instance.Decal->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));
		Instance.Decal->SetVisibility(true);

		// Setup Niagara (if available)
		if (NiagaraSystem)
		{
			Instance.Niagara->SetAsset(NiagaraSystem);
			Instance.Niagara->SetWorldLocation(CellLocation);
			Instance.Niagara->Activate(true);
		}
		else
		{
			Instance.Niagara->Deactivate();
		}

		ActiveIndices.Add(FreeIndex);
	}
}

void UGridHighlightComponent::ClearHighlights(EHighlightType HighlightType)
{
	TArray<int32>* ActiveIndices = ActiveHighlightIndices.Find(HighlightType);
	if (!ActiveIndices)
	{
		return;
	}

	for (int32 PoolIndex : *ActiveIndices)
	{
		FHighlightInstance& Instance = HighlightPool[PoolIndex];
		Instance.Decal->SetVisibility(false);
		Instance.Niagara->Deactivate();
		Instance.bInUse = false;
	}

	ActiveIndices->Empty();
}

void UGridHighlightComponent::ClearAllHighlights()
{
	for (auto& Pair : ActiveHighlightIndices)
	{
		for (int32 PoolIndex : Pair.Value)
		{
			FHighlightInstance& Instance = HighlightPool[PoolIndex];
			Instance.Decal->SetVisibility(false);
			Instance.Niagara->Deactivate();
			Instance.bInUse = false;
		}
	}

	ActiveHighlightIndices.Empty();
}

// Legacy API wrappers
void UGridHighlightComponent::ShowValidMoves(const TArray<FTacCoordinates>& ValidCells)
{
	ShowHighlights(ValidCells, EHighlightType::Movement);
}

void UGridHighlightComponent::ShowValidTargets(const TArray<FTacCoordinates>& TargetCells)
{
	ShowHighlights(TargetCells, EHighlightType::Attack);
}
