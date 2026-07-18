#pragma once
#include "CoreMinimal.h"
#include "Presentation/Core/Actions/MovePresentationAction.h"
#include "TacticalMovePresentationAction.generated.h"

// Tactical-aware move action: pushes bIsMoving to the unit's AnimBP on move start/stop.
UCLASS(BlueprintType, Blueprintable)
class KBS_API UTacticalMovePresentationAction : public UMovePresentationAction
{
	GENERATED_BODY()

protected:
	virtual void OnMoveStarted() override;
	virtual void OnMoveFinished() override;
};
