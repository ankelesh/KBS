#pragma once
#include "CoreMinimal.h"

struct FUnitMoveOffFieldPayload;
class UTacGridSubsystem;
class UMovePresentationAction;
class AUnit;

namespace TacticalLogConverters
{
	// Converts a unit's field exit into a move action running the unit off-field toward its own baseline.
	// UnitLookup must contain Payload.UnitId - a move-off-field payload referencing an untracked unit is a broken log contract.
	UMovePresentationAction* ConvertUnitMoveOffFieldPayload(const FUnitMoveOffFieldPayload& Payload, const TMap<FGuid, AUnit*>& UnitLookup, UTacGridSubsystem* GridSubsystem);
}
