#pragma once
#include "CoreMinimal.h"
#include "Presentation/Core/BasePresentationBuilder.h"
#include "TacticalPresentationBuilder.generated.h"

class UGridDataManager;
class UTacLogSubsystem;

UCLASS(BlueprintType, Blueprintable)
class KBS_API UTacticalPresentationBuilder : public UBasePresentationBuilder
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Presentation|Tactical")
	void SetGridDataManager(UGridDataManager* InGridDataManager);

	UFUNCTION(BlueprintCallable, Category = "Presentation|Tactical")
	void SetLogSubsystem(UTacLogSubsystem* InLogSubsystem);

protected:
	virtual UPresentationSequence* Build_Implementation(FGuid FromEventId, FGuid ToEventId) override;

private:
	UPROPERTY()
	TObjectPtr<UGridDataManager> GridDataManager;

	UPROPERTY()
	TObjectPtr<UTacLogSubsystem> LogSubsystem;
};
