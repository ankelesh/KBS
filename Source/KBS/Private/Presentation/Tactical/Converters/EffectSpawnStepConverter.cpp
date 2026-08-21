#include "Presentation/Tactical/Converters/EffectSpawnStepConverter.h"
#include "GameMechanics/Tactical/Grid/Subsystems/Logs/TacLogAbilitySteps.h"
#include "GameMechanics/Units/Unit.h"
#include "GameMechanics/Units/BattleEffects/BattleEffectDataAsset.h"
#include "Presentation/Tactical/Actions/FloatingTextPresentationAction.h"
#include "Engine/AssetManager.h"

UFloatingTextPresentationAction* TacticalLogConverters::ConvertEffectSpawnStep(const FTacLogEffectSpawnStep& Step, const FPresentationBuildContext& Context)
{
	AUnit* const* FoundUnit = Context.UnitLookup->Find(Step.TargetUnitId);
	checkf(FoundUnit, TEXT("Effect spawn step references unit %s - unit is present in UnitLookup"), *Step.TargetUnitId.ToString());

	// Asset must already be loaded - it was loaded to apply the effect during real gameplay.
	const UBattleEffectDataAsset* EffectAsset = Cast<UBattleEffectDataAsset>(
		UAssetManager::Get().GetPrimaryAssetObject(Step.EffectRef.AssetId));
	checkf(EffectAsset, TEXT("Effect spawn step references unloaded/invalid effect asset %s"), *Step.EffectRef.AssetId.ToString());

	const FLinearColor Color = [&]
	{
		if (Context.Config)
		{
			switch (EffectAsset->Polarity)
			{
			case EEffectPolarity::Positive: return Context.Config->EffectSpawnPositiveColor;
			case EEffectPolarity::Negative: return Context.Config->EffectSpawnNegativeColor;
			default: return Context.Config->EffectSpawnNeutralColor;
			}
		}
		switch (EffectAsset->Polarity)
		{
		case EEffectPolarity::Positive: return FLinearColor::Green;
		case EEffectPolarity::Negative: return FLinearColor::Red;
		default: return FLinearColor::White;
		}
	}();

	UFloatingTextPresentationAction* Action = NewObject<UFloatingTextPresentationAction>();
	Action->Context.Actor = *FoundUnit;
	Action->Context.Text = EffectAsset->Name;
	Action->Context.Color = Color;
	return Action;
}
