#include "Presentation/Tactical/Converters/EffectEndPayloadConverter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "Presentation/Tactical/Actions/FloatingTextPresentationAction.h"
#include "Engine/AssetManager.h"
#include "Internationalization/Text.h"

UFloatingTextPresentationAction* TacticalLogConverters::ConvertEffectEndPayload(const FEffectEndPayload& Payload, const TMap<FGuid, AUnit*>& UnitLookup)
{
	AUnit* const* FoundUnit = UnitLookup.Find(Payload.OwnerUnitId);
	checkf(FoundUnit, TEXT("Effect end payload references unit %s not found on grid"), *Payload.OwnerUnitId.ToString());

	// Asset must already be loaded - it was loaded to apply the effect during real gameplay.
	const UBattleEffectDataAsset* EffectAsset = Cast<UBattleEffectDataAsset>(
		UAssetManager::Get().GetPrimaryAssetObject(Payload.EffectAssetId));
	if (!EffectAsset)
	{
		return nullptr; // no backing asset (e.g. an innate status expiring, not a real effect)
	}

	FFormatNamedArguments Args;
	Args.Add(TEXT("EffectName"), EffectAsset->Name);

	UFloatingTextPresentationAction* Action = NewObject<UFloatingTextPresentationAction>();
	Action->Context.Actor = *FoundUnit;
	Action->Context.Text = FText::Format(NSLOCTEXT("EffectEndPayloadConverter", "EffectEndedFormat", "{EffectName} ended"), Args);
	Action->Context.Color = FLinearColor::Gray;
	return Action;
}
