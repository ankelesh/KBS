#include "GameMechanics/Tactical/Grid/Components/AIControllerComponent.h"
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
void UAIControllerComponent::Initialize(UGridDataManager* InDataManager,
                                        UGridMovementComponent* InMovement, UGridTargetingComponent* InTargeting,
                                        UAbilityExecutorComponent* InAbilityExecutor, UTurnManagerComponent* InTurnManager)
{
	DataManager = InDataManager;
	MovementComponent = InMovement;
	TargetingComponent = InTargeting;
	AbilityExecutor = InAbilityExecutor;
	TurnManager = InTurnManager;
}
void UAIControllerComponent::ExecuteAITurn(AUnit* AIUnit)
{
	if (!AIUnit || !DataManager || !TurnManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("AIController: Invalid AIUnit or components"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("AIController: Executing turn for %s"), *AIUnit->GetName());
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
	UE_LOG(LogTemp, Log, TEXT("AIController: No action possible, ending turn"));
	TurnManager->EndCurrentUnitTurn();
}
bool UAIControllerComponent::TryExecuteAttack(AUnit* AIUnit)
{
	if (!AIUnit || !TargetingComponent)
	{
		return false;
	}
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
	ETargetReach Reach = CurrentAbility->GetTargeting();
	TArray<AUnit*> ValidTargets = TargetingComponent->GetValidTargetUnits(AIUnit, Reach, false, false);
	if (ValidTargets.Num() == 0)
	{
		return false;
	}
	AUnit* BestTarget = FindBestTarget(AIUnit, ValidTargets);
	if (!BestTarget)
	{
		return false;
	}
	int32 TargetRow, TargetCol;
	EBattleLayer TargetLayer;
	if (!DataManager->GetUnitPosition(BestTarget, TargetRow, TargetCol, TargetLayer))
	{
		return false;
	}
	FIntPoint TargetCell(TargetCol, TargetRow);
	TArray<AUnit*> ResolvedTargets = TargetingComponent->ResolveTargetsFromClick(
		AIUnit, TargetCell, TargetLayer, Reach, nullptr
	);
	if (ResolvedTargets.Num() == 0)
	{
		return false;
	}
	TurnManager->ExecuteAbilityOnTargets(AIUnit, ResolvedTargets);
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
	if (!AIUnit || !DataManager || !MovementComponent)
	{
		return false;
	}
	UBattleTeam* EnemyTeam = DataManager->GetEnemyTeam(AIUnit);
	if (!EnemyTeam)
	{
		return false;
	}
	int32 AIRow, AICol;
	EBattleLayer AILayer;
	if (!DataManager->GetUnitPosition(AIUnit, AIRow, AICol, AILayer))
	{
		return false;
	}
	FVector AIPosition = DataManager->GetCellWorldLocation(AIRow, AICol, AILayer);
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
		if (!DataManager->GetUnitPosition(Enemy, EnemyRow, EnemyCol, EnemyLayer))
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
	FIntPoint BestMove = FindBestMovePosition(AIUnit, ClosestEnemy);
	if (BestMove.X < 0 || BestMove.Y < 0)
	{
		return false;
	}
	bool bMoveSuccess = MovementComponent->MoveUnit(AIUnit, BestMove.Y, BestMove.X);
	return bMoveSuccess;
}
FIntPoint UAIControllerComponent::FindBestMovePosition(AUnit* AIUnit, AUnit* TargetEnemy)
{
	if (!AIUnit || !TargetEnemy || !MovementComponent || !DataManager)
	{
		return FIntPoint(-1, -1);
	}
	TArray<FIntPoint> ValidMoves = TargetingComponent->GetValidTargetCells(AIUnit, ETargetReach::Movement);
	if (ValidMoves.Num() == 0)
	{
		return FIntPoint(-1, -1);
	}
	int32 TargetRow, TargetCol;
	EBattleLayer TargetLayer;
	if (!DataManager->GetUnitPosition(TargetEnemy, TargetRow, TargetCol, TargetLayer))
	{
		return FIntPoint(-1, -1);
	}
	FVector TargetPosition = DataManager->GetCellWorldLocation(TargetRow, TargetCol, TargetLayer);
	int32 AIRow, AICol;
	EBattleLayer AILayer;
	if (!DataManager->GetUnitPosition(AIUnit, AIRow, AICol, AILayer))
	{
		return FIntPoint(-1, -1);
	}
	FIntPoint BestMove(-1, -1);
	float BestDistance = MAX_FLT;
	for (const FIntPoint& MoveCell : ValidMoves)
	{
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
