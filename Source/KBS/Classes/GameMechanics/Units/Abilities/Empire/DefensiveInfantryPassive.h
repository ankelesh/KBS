#pragma once
#include "CoreMinimal.h"
#include "GameMechanics/Units/Abilities/Passives/BattleStartPassive.h"
#include "DefensiveInfantryPassive.generated.h"

/** Passive: the owner enters the Defending status when the battle starts (or on spawn if it already started). */
UCLASS()
class KBS_API UDefensiveInfantryPassive : public UBattleStartPassive
{
	GENERATED_BODY()
protected:
	virtual void OnBattleStart() override;
};
