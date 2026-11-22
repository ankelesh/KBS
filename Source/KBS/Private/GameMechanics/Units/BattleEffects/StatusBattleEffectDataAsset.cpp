#include "GameMechanics/Units/BattleEffects/StatusBattleEffectDataAsset.h"

UStatusBattleEffectDataAsset::UStatusBattleEffectDataAsset()
{
	Name = NSLOCTEXT("KBS", "ParalysisName", "Paralysis");
	Description = NSLOCTEXT("KBS", "ParalysisDescription", "The unit cannot act.");
	Polarity = EEffectPolarity::Negative;
	StackingId = TEXT("Paralysis");
	StackPolicy = EEffectStackPolicy::RefreshOld;
}
