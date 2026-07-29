#pragma once
#include "CoreMinimal.h"
#include "GameplayTypes/TeamConstants.h"
#include "GameplayTypes/GridCoordinates.h"
#include "TacticalBattleSetup.generated.h"

class UUnitDefinition;

USTRUCT()
struct FBattleUnitPlacement
{
	GENERATED_BODY()
	UPROPERTY() TObjectPtr<UUnitDefinition> Definition;
	UPROPERTY() int32 TeamRow = 0; // 0..2, team-local row
	UPROPERTY() int32 TeamCol = 0; // 0..1, team-local col
	UPROPERTY() ETacGridLayer Layer = ETacGridLayer::Ground;
};

USTRUCT()
struct FTacticalBattleSetup
{
	GENERATED_BODY()
	UPROPERTY() TArray<FBattleUnitPlacement> AttackerUnits;
	UPROPERTY() TArray<FBattleUnitPlacement> DefenderUnits;
	UPROPERTY() ETeamSide AIControlledTeam = ETeamSide::Defender;
	UPROPERTY() FName ReturnLevelName;

	bool IsValid() const { return AttackerUnits.Num() > 0 || DefenderUnits.Num() > 0; }
	ETeamSide GetPlayerTeamSide() const
	{
		return AIControlledTeam == ETeamSide::Attacker ? ETeamSide::Defender : ETeamSide::Attacker;
	}
};
