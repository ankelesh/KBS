#pragma once
#include "UI/Common/Controller/KbsBaseController.h"
#include "TacticalGameController.generated.h"

class UTacticalHUD;
class UTacGridSubsystem;

UCLASS()
class KBS_API ATacticalGameController : public AKbsBaseController
{
	GENERATED_BODY()

public:
	UTacticalHUD* GetTacticalHUD() const { return TacticalHUD; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	TSubclassOf<UTacticalHUD> TacticalHUDClass;

private:
	UFUNCTION()
	void OnGridSubsystemReady(UTacGridSubsystem* Subsystem);

	UPROPERTY()
	TObjectPtr<UTacticalHUD> TacticalHUD;
};
