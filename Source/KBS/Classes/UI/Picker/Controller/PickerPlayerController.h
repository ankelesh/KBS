#pragma once
#include "CoreMinimal.h"
#include "UI/Common/Controller/KbsBaseController.h"
#include "PickerPlayerController.generated.h"

class UPickerHUD;

UCLASS()
class KBS_API APickerPlayerController : public AKbsBaseController
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly) TSubclassOf<UPickerHUD> PickerHUDClass;

private:
	UPROPERTY() TObjectPtr<UPickerHUD> PickerHUD;
};
