#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GridMovementComponent.generated.h"
class AUnit;
class ATacBattleGrid;
class UBattleTeam;
class UGridDataManager;
enum class EBattleLayer : uint8;
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnUnitFlankStateChanged, AUnit*  , bool  , FIntPoint  );
USTRUCT()
struct FMovementInterpData
{
	GENERATED_BODY()
	FVector StartLocation = FVector::ZeroVector;
	FVector TargetLocation = FVector::ZeroVector;
	FVector Direction = FVector::ZeroVector;
	float ElapsedTime = 0.0f;
	float Duration = 0.3f;
	bool bNeedsFinalRotation = false;
	FRotator FinalRotation = FRotator::ZeroRotator;
	ETeamSide UnitTeamSide = ETeamSide::Attacker;
};
UCLASS()
class KBS_API UGridMovementComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UGridMovementComponent();
	void Initialize(ATacBattleGrid* InGrid, UGridDataManager* InDataManager);
	FOnUnitFlankStateChanged OnUnitFlankStateChanged;
	bool MoveUnit(AUnit* Unit, int32 TargetRow, int32 TargetCol);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	static constexpr float ModelForwardOffset = -90.0f;  
	static constexpr float AttackerDefaultYaw = 0.0f;    
	static constexpr float DefenderDefaultYaw = 180.0f;  
private:
	UPROPERTY()
	TObjectPtr<ATacBattleGrid> Grid;
	UPROPERTY()
	TObjectPtr<UGridDataManager> DataManager;
	TMap<TObjectPtr<AUnit>, FMovementInterpData> UnitsBeingMoved;
	void StartMovementInterpolation(AUnit* Unit, FVector StartLocation, FVector TargetLocation, FVector Direction, float Distance, float Speed, int32 TargetRow, int32 TargetCol);
	FRotator CalculateDefaultCellOrientation(int32 Row, int32 Col, ETeamSide TeamSide) const;
	FIntPoint FindClosestNonFlankCell(int32 FlankRow, int32 FlankCol) const;
	void ApplyFlankRotation(AUnit* Unit, int32 Row, int32 Col);
	void RestoreOriginalRotation(AUnit* Unit);
};
