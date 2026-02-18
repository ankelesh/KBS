#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TacSubsystemControl.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllSubsystemsReadyForStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTurnReadyForStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGridReadyForStart);
class UTacGridSubsystem;
class UTacTurnSubsystem;
UCLASS()
class UTacSubsystemControl : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	FOnAllSubsystemsReadyForStart ReadyForStart;
	FOnTurnReadyForStart TurnReadyForStart;
	FOnGridReadyForStart GridReadyForStart;

	UTacSubsystemControl() : bGridReadyForStart(false), bTurnReadyForStart(false) {}

	void NotifyGridReady();
	void NotifyTurnReady();

	UFUNCTION()
	bool IsReadyForStart() const { return bTurnReadyForStart && bGridReadyForStart; }

	void StartBattle();
private:
	bool bGridReadyForStart;
	bool bTurnReadyForStart;
	bool CheckReady();
};
