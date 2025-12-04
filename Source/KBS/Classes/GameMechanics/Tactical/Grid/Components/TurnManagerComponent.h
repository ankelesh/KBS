#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurnManagerComponent.generated.h"

class AUnit;
class UBattleTeam;

USTRUCT(BlueprintType)
struct FTurnQueueEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AUnit> Unit;

	UPROPERTY()
	int32 InitiativeRoll;

	UPROPERTY()
	int32 TotalInitiative;

	bool operator<(const FTurnQueueEntry& Other) const
	{
		return TotalInitiative > Other.TotalInitiative; // Descending sort
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGlobalTurnStarted, int32, TurnNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGlobalTurnEnded, int32, TurnNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitTurnStart, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitTurnEnd, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleEnded, UBattleTeam*, Winner);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KBS_API UTurnManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTurnManagerComponent();

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
	TArray<FTurnQueueEntry> TurnQueue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
	TObjectPtr<AUnit> ActiveUnit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
	int32 CurrentTurnNumber = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
	bool bBattleActive = false;

	// Config
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn")
	int32 InitiativeRollMin = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn")
	int32 InitiativeRollMax = 20;

	// Unit tracking
	UPROPERTY()
	TArray<TObjectPtr<AUnit>> AllUnits;

	UPROPERTY()
	TObjectPtr<UBattleTeam> AttackerTeam;

	UPROPERTY()
	TObjectPtr<UBattleTeam> DefenderTeam;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Turn Events")
	FOnGlobalTurnStarted OnGlobalTurnStarted;

	UPROPERTY(BlueprintAssignable, Category = "Turn Events")
	FOnGlobalTurnEnded OnGlobalTurnEnded;

	UPROPERTY(BlueprintAssignable, Category = "Turn Events")
	FOnUnitTurnStart OnUnitTurnStart;

	UPROPERTY(BlueprintAssignable, Category = "Turn Events")
	FOnUnitTurnEnd OnUnitTurnEnd;

	UPROPERTY(BlueprintAssignable, Category = "Turn Events")
	FOnBattleEnded OnBattleEnded;

	// Battle lifecycle
	UFUNCTION(BlueprintCallable, Category = "Turn")
	void StartBattle(const TArray<AUnit*>& Units);

	UFUNCTION(BlueprintCallable, Category = "Turn")
	void EndBattle();

	// Turn flow
	void StartGlobalTurn();
	void EndGlobalTurn();
	void ActivateNextUnit();
	void EndCurrentUnitTurn();

	// Queue management
	void BuildInitiativeQueue();
	void InsertUnitIntoQueue(AUnit* Unit);
	void RemoveUnitFromQueue(AUnit* Unit);

	// Validation
	UFUNCTION(BlueprintCallable, Category = "Turn")
	bool CanUnitAct(AUnit* Unit) const;

	bool BattleIsOver() const;

	// Helpers
	int32 RollInitiative() const;
};
