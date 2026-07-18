#include "Presentation/Tactical/Converters/TacEffectPayloadConverter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogPayloads.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "Presentation/Core/Actions/VfxPresentationAction.h"
#include "Engine/AssetManager.h"
#include "NiagaraSystem.h"

UVfxPresentationAction* TacticalLogConverters::ConvertTacEffectPayload(const FTacEffectPayload& Payload, const TMap<FGuid, AUnit*>& UnitLookup)
{
	AUnit* const* FoundUnit = UnitLookup.Find(Payload.OwnerUnitId);
	checkf(FoundUnit, TEXT("Effect trigger payload references unit %s not found on grid"), *Payload.OwnerUnitId.ToString());

	const UBattleEffectDataAsset* EffectAsset = Cast<UBattleEffectDataAsset>(
		UAssetManager::Get().GetPrimaryAssetObject(Payload.EffectAssetId));
	if (!EffectAsset || EffectAsset->AppliedVFX.IsNull())
	{
		return nullptr; // not every effect trigger has a VFX (e.g. innate statuses)
	}

	UNiagaraSystem* Vfx = EffectAsset->AppliedVFX.LoadSynchronous();
	if (!Vfx)
	{
		return nullptr;
	}

	AUnit* OwnerUnit = *FoundUnit;
	UVfxPresentationAction* Action = NewObject<UVfxPresentationAction>();
	Action->Context.Actor = OwnerUnit;
	Action->Context.System = Vfx;
	Action->Context.Duration = EffectAsset->VFXDuration;
	Action->Context.ZOffset = OwnerUnit->GetSimpleCollisionHalfHeight();
	return Action;
}
