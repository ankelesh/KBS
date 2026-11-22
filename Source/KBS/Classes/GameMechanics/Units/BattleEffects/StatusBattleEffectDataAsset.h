#pragma once
#include "CoreMinimal.h"
#include "BattleEffectDataAsset.h"
#include "GameMechanics/Units/Stats/UnitStatusContainer.h"
#include "StatusBattleEffectDataAsset.generated.h"

/** Effect that holds a ref-counted unit status (TurnBlocked, Pinned, Silenced, Disoriented) while it lives.
 *  Name/Description/Image come from the base asset; the defaults describe Paralysis. */
UCLASS(BlueprintType)
class KBS_API UStatusBattleEffectDataAsset : public UBattleEffectDataAsset
{
	GENERATED_BODY()
public:
	UStatusBattleEffectDataAsset();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status Effect")
	EUnitStatus Status = EUnitStatus::TurnBlocked;

	/** Ignores DefaultDuration; the effect stays until removed (e.g. dispelled). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status Effect")
	bool bPermanent = false;
};
