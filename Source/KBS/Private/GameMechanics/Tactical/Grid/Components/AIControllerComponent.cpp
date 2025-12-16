#include "GameMechanics/Tactical/Grid/Components/AIControllerComponent.h"
#include "GameMechanics/Tactical/Grid/TacBattleGrid.h"
#include "GameMechanics/Tactical/Grid/Components/GridDataManager.h"
#include "GameMechanics/Tactical/Grid/Components/GridMovementComponent.h"
#include "GameMechanics/Tactical/Grid/Components/GridTargetingComponent.h"
#include "GameMechanics/Tactical/Grid/Components/AbilityExecutorComponent.h"
#include "GameMechanics/Tactical/Grid/Components/TurnManagerComponent.h"
#include "GameMechanics/Tactical/DamageCalculator.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/Abilities/AbilityInventoryComponent.h"
#include "GameMechanics/Units/Abilities/UnitAbilityInstance.h"
#include "GameplayTypes/DamageTypes.h"
#include "GameplayTypes/CombatTypes.h"

UAIControllerComponent::UAIControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAIControllerComponent::Initialize(ATacBattleGrid* InGrid, UGridDataManager* InDataManager,
                                        UGridMovementComponent* InMovement, UGridTargetingComponent* InTargeting,
                                        UAbilityExecutorComponent* InAbilityExecutor)
{
	Grid = InGrid;
	DataManager = InDataManager;
	MovementComponent = InMovement;
	TargetingComponent = InTargeting;
	AbilityExecutor = InAbilityExecutor;
}

void UAIControllerComponent::ExecuteAITurn(AUnit* AIUnit)
{
	if (!AIUnit || !Grid)
	{
		UE_LOG(LogTemp, Warning, TEXT("AIController: Invalid AIUnit or Grid"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("AIController: Executing turn for %s"), *AIUnit->GetName());

	// Priority chain: Attack -> Move -> EndTurn
	if (TryExecuteAttack(AIUnit))
	{
		UE_LOG(LogTemp, Log, TEXT("AIController: Attack executed"));
		return;
	}

	if (TryMoveTowardEnemy(AIUnit))
	{
		UE_LOG(LogTemp, Log, TEXT("AIController: Movement executed"));
		return;
	}

	// No action possible - end turn
	UE_LOG(LogTemp, Log, TEXT("AIController: No action possible, ending turn"));
	Grid->GetTurnManager()->EndCurrentUnitTurn();
}

bool UAIControllerComponent::TryExecuteAttack(AUnit* AIUnit)
{
	if (!AIUnit || !TargetingComponent || !Grid)
	{
		return false;
	}

	// Get current active ability
	UAbilityInventoryComponent* AbilityInventory = AIUnit->AbilityInventory;
	if (!AbilityInventory)
	{
		return false;
	}

	UUnitAbilityInstance* CurrentAbility = AbilityInventory->GetCurrentActiveAbility();
	if (!CurrentAbility)
	{
		return false;
	}

	// Get ability's targeting type
	ETargetReach Reach = CurrentAbility->GetTargeting();

	// Query valid targets
	TArray<AUnit*> ValidTargets = TargetingComponent->GetValidTargetUnits(AIUnit, Reach, false, false);

	if (ValidTargets.Num() == 0)
	{
		return false;
	}

	// Find best target
	AUnit* BestTarget = FindBestTarget(AIUnit, ValidTargets);
	if (!BestTarget)
	{
		return false;
	}

	// Get target's grid position
	int32 TargetRow, TargetCol;
	EBattleLayer TargetLayer;
	if (!Grid->GetUnitPosition(BestTarget, TargetRow, TargetCol, TargetLayer))
	{
		return false;
	}

	FIntPoint TargetCell(TargetCol, TargetRow);

	// Resolve targets from click (handles Area/AllEnemies targeting)
	TArray<AUnit*> ResolvedTargets = TargetingComponent->ResolveTargetsFromClick(
		AIUnit, TargetCell, TargetLayer, Reach, nullptr
	);

	if (ResolvedTargets.Num() == 0)
	{
		return false;
	}

	// Execute ability
	Grid->AbilityTargetSelected(AIUnit, ResolvedTargets);
	return true;
}

AUnit* UAIControllerComponent::FindBestTarget(AUnit* AIUnit, const TArray<AUnit*>& PotentialTargets)
{
	if (!AIUnit || PotentialTargets.Num() == 0)
	{
		return nullptr;
	}

	UDamageCalculator* DamageCalc = GetWorld()->GetSubsystem<UDamageCalculator>();
	if (!DamageCalc)
	{
		return nullptr;
	}

	// Get ability's targeting type for damage preview
	UAbilityInventoryComponent* AbilityInventory = AIUnit->AbilityInventory;
	if (!AbilityInventory)
	{
		return nullptr;
	}

	UUnitAbilityInstance* CurrentAbility = AbilityInventory->GetCurrentActiveAbility();
	if (!CurrentAbility)
	{
		return nullptr;
	}

	ETargetReach Reach = CurrentAbility->GetTargeting();

	AUnit* BestTarget = nullptr;
	int32 BestDamage = -1;

	for (AUnit* Target : PotentialTargets)
	{
		if (!Target || Target->IsDead())
		{
			continue;
		}

		// Preview damage
		FPreviewHitResult Preview = DamageCalc->PreviewDamage(AIUnit, Target, Reach);
		int32 ExpectedDamage = Preview.DamageResult.Damage;

		if (ExpectedDamage > BestDamage)
		{
			BestDamage = ExpectedDamage;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

bool UAIControllerComponent::TryMoveTowardEnemy(AUnit* AIUnit)
{
	if (!AIUnit || !Grid || !DataManager || !MovementComponent)
	{
		return false;
	}

	// Get enemy team
	UBattleTeam* EnemyTeam = Grid->GetEnemyTeam(AIUnit);
	if (!EnemyTeam)
	{
		return false;
	}

	// Get AI unit's current position
	int32 AIRow, AICol;
	EBattleLayer AILayer;
	if (!Grid->GetUnitPosition(AIUnit, AIRow, AICol, AILayer))
	{
		return false;
	}

	FVector AIPosition = DataManager->GetCellWorldLocation(AIRow, AICol, AILayer);

	// Find closest living enemy
	AUnit* ClosestEnemy = nullptr;
	float ClosestDistance = MAX_FLT;

	const TArray<TObjectPtr<AUnit>>& EnemyUnits = EnemyTeam->GetUnits();
	for (AUnit* Enemy : EnemyUnits)
	{
		if (!Enemy || Enemy->IsDead())
		{
			continue;
		}

		int32 EnemyRow, EnemyCol;
		EBattleLayer EnemyLayer;
		if (!Grid->GetUnitPosition(Enemy, EnemyRow, EnemyCol, EnemyLayer))
		{
			continue;
		}

		FVector EnemyPosition = DataManager->GetCellWorldLocation(EnemyRow, EnemyCol, EnemyLayer);
		float Distance = FVector::Dist(AIPosition, EnemyPosition);

		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestEnemy = Enemy;
		}
	}

	if (!ClosestEnemy)
	{
		return false;
	}

	// Find best move position
	FIntPoint BestMove = FindBestMovePosition(AIUnit, ClosestEnemy);
	if (BestMove.X < 0 || BestMove.Y < 0)
	{
		return false;
	}

	// Move unit (BestMove is (Col, Row) format as FIntPoint)
	bool bMoveSuccess = Grid->MoveUnit(AIUnit, BestMove.Y, BestMove.X);
	return bMoveSuccess;
}

FIntPoint UAIControllerComponent::FindBestMovePosition(AUnit* AIUnit, AUnit* TargetEnemy)
{
	if (!AIUnit || !TargetEnemy || !MovementComponent || !Grid || !DataManager)
	{
		return FIntPoint(-1, -1);
	}

	// Get valid moves
	TArray<FIntPoint> ValidMoves = MovementComponent->GetValidMoveCells(AIUnit);
	if (ValidMoves.Num() == 0)
	{
		return FIntPoint(-1, -1);
	}

	// Get target enemy position
	int32 TargetRow, TargetCol;
	EBattleLayer TargetLayer;
	if (!Grid->GetUnitPosition(TargetEnemy, TargetRow, TargetCol, TargetLayer))
	{
		return FIntPoint(-1, -1);
	}

	FVector TargetPosition = DataManager->GetCellWorldLocation(TargetRow, TargetCol, TargetLayer);

	// Get AI unit's layer for world location calculation
	int32 AIRow, AICol;
	EBattleLayer AILayer;
	if (!Grid->GetUnitPosition(AIUnit, AIRow, AICol, AILayer))
	{
		return FIntPoint(-1, -1);
	}

	// Find move cell closest to target
	FIntPoint BestMove(-1, -1);
	float BestDistance = MAX_FLT;

	for (const FIntPoint& MoveCell : ValidMoves)
	{
		// MoveCell is in (Col, Row) format
		FVector MoveCellPosition = DataManager->GetCellWorldLocation(MoveCell.Y, MoveCell.X, AILayer);
		float Distance = FVector::Dist(MoveCellPosition, TargetPosition);

		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			BestMove = MoveCell;
		}
	}

	return BestMove;
}
