#include "Presentation/Tactical/Converters/UnitSpawnPayloadConverter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Units/Unit.h"
#include "Presentation/Tactical/Actions/FloatingTextPresentationAction.h"

UFloatingTextPresentationAction* TacticalLogConverters::ConvertUnitSpawnPayload(const FUnitSpawnPayload& Payload, const TMap<FGuid, AUnit*>& UnitLookup)
{
	AUnit* const* FoundUnit = UnitLookup.Find(Payload.SpawnedUnitId);
	checkf(FoundUnit, TEXT("Unit spawn payload references unit %s not found on grid"), *Payload.SpawnedUnitId.ToString());

	UFloatingTextPresentationAction* Action = NewObject<UFloatingTextPresentationAction>();
	Action->Context.Actor = *FoundUnit;
	Action->Context.Text = NSLOCTEXT("UnitSpawnPayloadConverter", "Spawned", "Spawned");
	Action->Context.Color = FLinearColor::White;
	return Action;
}
