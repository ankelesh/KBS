#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TacGridInputRouter.generated.h"
class AUnit;
class ATacBattleGrid;
class UGridDataManager;
class UGridMovementComponent;
class UGridTargetingComponent;
class UTurnManagerComponent;
enum class EBattleLayer : uint8;
UCLASS()
class KBS_API UTacGridInputRouter : public UActorComponent
{
	GENERATED_BODY()
public:
	UTacGridInputRouter();
	void Initialize(
		ATacBattleGrid* InGrid,
		UGridDataManager* InDataManager,
		UGridMovementComponent* InMovementComponent,
		UGridTargetingComponent* InTargetingComponent,
		UTurnManagerComponent* InTurnManager
	);
	void HandleGridClick(FKey ButtonPressed);
private:
	UPROPERTY()
	TObjectPtr<ATacBattleGrid> Grid;
	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;
	UPROPERTY()
	TObjectPtr<UGridMovementComponent> MovementComponent;
	UPROPERTY()
	TObjectPtr<UGridTargetingComponent> TargetingComponent;
	UPROPERTY()
	TObjectPtr<UTurnManagerComponent> TurnManager;

	bool GetCellUnderMouse(int32& OutRow, int32& OutCol, EBattleLayer& OutLayer) const;
};
