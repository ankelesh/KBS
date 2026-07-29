#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameplayTypes/TacticalBattleSetup.h"
#include "KbsGameInstance.generated.h"

UCLASS()
class KBS_API UKbsGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	void SetPendingBattle(const FTacticalBattleSetup& Setup);
	void ClearPendingBattle();
	bool HasPendingBattle() const { return PendingBattle.IsValid(); }
	const FTacticalBattleSetup& GetPendingBattle() const { return PendingBattle; }

private:
	UPROPERTY()
	FTacticalBattleSetup PendingBattle;
};
