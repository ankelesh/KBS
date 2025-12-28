#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameMechanics/Units/UnitDisplayData.h"
#include "GameplayTypes/AbilityTypes.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "TurnManagerComponent.generated.h"
class AUnit;
class UBattleTeam;
class UGridTargetingComponent;
class UGridMovementComponent;

UENUM(BlueprintType)
enum class EBattleState : uint8
{
	Idle,
	PlayerTurn,
	EnemyTurn,
	TurnTransition,
	RoundTransition
};

UENUM(BlueprintType)
enum class EPlayerTurnSubstate : uint8
{
	None,
	AwaitingInput,        // Input ENABLED
	ProcessingAbility,    // Input BLOCKED
	PlayingPresentation   // Input BLOCKED
};

UENUM(BlueprintType)
enum class EEnemyTurnSubstate : uint8
{
	None,
	Thinking,             // Input BLOCKED
	ExecutingAction,      // Input BLOCKED
	PlayingPresentation   // Input BLOCKED
};

USTRUCT(BlueprintType)
struct FTurnQueueEntry
{
	GENERATED_BODY()
	UPROPERTY()
	TObjectPtr<AUnit> Unit;
	UPROPERTY()
	int32 InitiativeRoll = 0;
	UPROPERTY()
	int32 TotalInitiative = 0;
	bool operator<(const FTurnQueueEntry& Other) const
	{
		return TotalInitiative > Other.TotalInitiative;  
	}
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGlobalTurnStarted, int32, TurnNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGlobalTurnEnded, int32, TurnNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitTurnStart, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitTurnEnd, AUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBattleEnded, bool, bHasWinner, ETeamSide, WinningSide);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBattleStateChanged, EBattleState, NewBattleState, EPlayerTurnSubstate, NewPlayerSubstate, EEnemyTurnSubstate, NewEnemySubstate);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KBS_API UTurnManagerComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UTurnManagerComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
	TArray<FTurnQueueEntry> TurnQueue;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
	TObjectPtr<AUnit> ActiveUnit;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
	int32 ActiveUnitInitiative;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
	int32 CurrentTurnNumber = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
	bool bBattleActive = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn")
	int32 InitiativeRollMin = -4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn")
	int32 InitiativeRollMax = 4;
	UPROPERTY()
	TArray<TObjectPtr<AUnit>> AllUnits;
	UPROPERTY()
	TObjectPtr<UBattleTeam> AttackerTeam;
	UPROPERTY()
	TObjectPtr<UBattleTeam> DefenderTeam;
	UPROPERTY()
	TObjectPtr<class UAbilityExecutorComponent> AbilityExecutor;
	UPROPERTY()
	TObjectPtr<class UGridHighlightComponent> HighlightComponent;
	UPROPERTY()
	TObjectPtr<UGridTargetingComponent> TargetingComponent;
	UPROPERTY()
	TObjectPtr<UGridMovementComponent> MovementComponent;
	UPROPERTY()
	ETeamSide PlayerControlledTeam = ETeamSide::Attacker;

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
	UPROPERTY(BlueprintAssignable, Category = "Turn Events")
	FOnBattleStateChanged OnBattleStateChanged;

	UFUNCTION(BlueprintCallable, Category = "Turn")
	AUnit* GetActiveUnit() const { return ActiveUnit; }
	UFUNCTION(BlueprintCallable, Category = "Turn")
	void StartBattle(const TArray<AUnit*>& Units);
	UFUNCTION(BlueprintCallable, Category = "Turn")
	void EndBattle();
	void StartGlobalTurn();
	void EndGlobalTurn();
	void ActivateNextUnit();
	UFUNCTION()
	void EndCurrentUnitTurn();
	void BuildInitiativeQueue();
	void InsertUnitIntoQueue(AUnit* Unit);
	void RemoveUnitFromQueue(AUnit* Unit);
	void WaitCurrentUnit();
	UFUNCTION(BlueprintCallable, Category = "Turn")
	bool CanUnitAct(AUnit* Unit) const;
	bool BattleIsOver() const;
	void HandleAbilityComplete(const FAbilityResult& Result);
	UFUNCTION(BlueprintCallable, Category = "Turn")
	void SwitchAbility(class UUnitAbilityInstance* NewAbility);
	void ExecuteAbilityOnTargets(AUnit* SourceUnit, const TArray<AUnit*>& Targets);
	void ExecuteAbilityOnTargets(AUnit* SourceUnit, const TArray<AUnit*>& Targets, FIntPoint ClickedCell, uint8 ClickedLayer);
	void ExecuteAbilityOnSelf(AUnit* SourceUnit, class UUnitAbilityInstance* Ability);
	UFUNCTION(BlueprintCallable, Category = "Turn")
	TArray<FUnitTurnQueueDisplay> GetQueueDisplayData() const;
	UFUNCTION(BlueprintCallable, Category = "Turn")
	FUnitTurnQueueDisplay GetActiveUnitDisplayData() const;

	// State Machine - Input control
	bool CanAcceptInput() const;

	// State Machine - Accessors
	EBattleState GetBattleState() const { return CurrentState; }
	EPlayerTurnSubstate GetPlayerSubstate() const { return PlayerSubstate; }
	EEnemyTurnSubstate GetEnemySubstate() const { return EnemySubstate; }
	FString GetCurrentStateName() const;

private:
	int32 RollInitiative() const;
	FUnitTurnQueueDisplay MakeUnitTurnQueueDisplay(AUnit* Unit, int32 Initiative, bool bIsActiveUnit) const;
	UPROPERTY()
	bool bWaitingForPresentation = false;
	UFUNCTION()
	void OnPresentationComplete();

	// State Machine - Members
	EBattleState CurrentState;
	EPlayerTurnSubstate PlayerSubstate;
	EEnemyTurnSubstate EnemySubstate;
	float StateEnterTime;
	float MaxStateTimeout;

	// State Machine - Methods
	bool IsValidTransition(EBattleState NewState, EPlayerTurnSubstate NewPlayerSubstate, EEnemyTurnSubstate NewEnemySubstate) const;
	void TransitionToState(EBattleState NewState, EPlayerTurnSubstate NewPlayerSubstate = EPlayerTurnSubstate::None, EEnemyTurnSubstate NewEnemySubstate = EEnemyTurnSubstate::None);
	FString GetStateName(EBattleState State, EPlayerTurnSubstate PlayerSub, EEnemyTurnSubstate EnemySub) const;
};
