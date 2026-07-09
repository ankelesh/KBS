#include "Presentation/Tactical/Converters/EffectSpawnStepConverter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "Presentation/Tactical/Actions/FloatingTextPresentationAction.h"
#include "Engine/AssetManager.h"

namespace
{
	FLinearColor ColorForPolarity(EEffectPolarity Polarity)
	{
		switch (Polarity)
		{
		case EEffectPolarity::Positive: return FLinearColor::Green;
		case EEffectPolarity::Negative: return FLinearColor::Red;
		default: return FLinearColor::White;
		}
	}
}

UFloatingTextPresentationAction* TacticalLogConverters::ConvertEffectSpawnStep(const FTacLogEffectSpawnStep& Step, const TMap<FGuid, AUnit*>& UnitLookup)
{
	AUnit* const* FoundUnit = UnitLookup.Find(Step.TargetUnitId);
	checkf(FoundUnit, TEXT("Effect spawn step references unit %s not found on grid"), *Step.TargetUnitId.ToString());

	// Asset must already be loaded - it was loaded to apply the effect during real gameplay.
	const UBattleEffectDataAsset* EffectAsset = Cast<UBattleEffectDataAsset>(
		UAssetManager::Get().GetPrimaryAssetObject(Step.EffectRef.AssetId));
	checkf(EffectAsset, TEXT("Effect spawn step references unloaded/invalid effect asset %s"), *Step.EffectRef.AssetId.ToString());

	UFloatingTextPresentationAction* Action = NewObject<UFloatingTextPresentationAction>();
	Action->Context.Actor = *FoundUnit;
	Action->Context.Text = EffectAsset->Name;
	Action->Context.Color = ColorForPolarity(EffectAsset->Polarity);
	return Action;
}
