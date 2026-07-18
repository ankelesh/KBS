#include "Presentation/Tactical/Converters/UnitDespawnPayloadConverter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Units/Unit.h"
#include "Presentation/Tactical/Actions/DespawnPresentationAction.h"

UDespawnPresentationAction* TacticalLogConverters::ConvertUnitDespawnPayload(const FUnitDespawnPayload& Payload, const TMap<FGuid, AUnit*>& UnitLookup)
{
	AUnit* const* FoundUnit = UnitLookup.Find(Payload.UnitId);
	checkf(FoundUnit, TEXT("Unit despawn payload references unit %s not found on grid"), *Payload.UnitId.ToString());

	UDespawnPresentationAction* Action = NewObject<UDespawnPresentationAction>();
	Action->Context.Unit = *FoundUnit;
	return Action;
}
