#include "Presentation/Tactical/Converters/UnitSpawnPayloadConverter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Units/Unit.h"
#include "Presentation/Tactical/Actions/FloatingTextPresentationAction.h"

UFloatingTextPresentationAction* TacticalLogConverters::ConvertUnitSpawnPayload(const FUnitSpawnPayload& Payload, const FPresentationBuildContext& Context)
{
	AUnit* const* FoundUnit = Context.UnitLookup->Find(Payload.SpawnedUnitId);
	checkf(FoundUnit, TEXT("Unit spawn payload references unit %s - unit is present in UnitLookup"), *Payload.SpawnedUnitId.ToString());

	const FLinearColor Color = Context.Config ? Context.Config->UnitSpawnedColor : FLinearColor::White;

	UFloatingTextPresentationAction* Action = NewObject<UFloatingTextPresentationAction>();
	Action->Context.Actor = *FoundUnit;
	Action->Context.Text = NSLOCTEXT("UnitSpawnPayloadConverter", "Spawned", "Spawned");
	Action->Context.Color = Color;
	return Action;
}
