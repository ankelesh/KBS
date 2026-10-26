#pragma once
#include "CoreMinimal.h"
#include "Presentation/Core/BasePresentationBuilder.h"
#include "Engine/StreamableManager.h"
#include "TacticalPresentationBuilder.generated.h"

class UTacGridSubsystem;
class UTacLogSubsystem;
class UTacticalPresentationBuilderConfig;

UCLASS(BlueprintType, Blueprintable)
class KBS_API UTacticalPresentationBuilder : public UBasePresentationBuilder
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Presentation|Tactical")
	void SetGridSubsystem(UTacGridSubsystem* InGridSubsystem);

	UFUNCTION(BlueprintCallable, Category = "Presentation|Tactical")
	void SetLogSubsystem(UTacLogSubsystem* InLogSubsystem);

	// Optional. Null means every action falls back to its own hardcoded defaults.
	UFUNCTION(BlueprintCallable, Category = "Presentation|Tactical")
	void SetConfig(UTacticalPresentationBuilderConfig* InConfig);

	// Async-preloads all presentation manifests and their visual content.
	// AbilityAssetIds: ability primary asset ids for the battle's full ability set.
	// WeaponDeliveryPaths: soft paths to UAbilityPresentationAsset from weapon DefaultDelivery fields.
	// OnComplete fires when all content is resident; Build() must not be called before it fires.
	UFUNCTION(BlueprintCallable, Category = "Presentation|Tactical")
	void BeginPreload(const TArray<FPrimaryAssetId>& AbilityAssetIds, const TArray<FSoftObjectPath>& WeaponDeliveryPaths, FSimpleDelegate OnComplete);

protected:
	virtual UPresentationSequence* Build_Implementation(FGuid FromEventId, FGuid ToEventId) override;

private:
	UPROPERTY()
	TObjectPtr<UTacGridSubsystem> GridSubsystem;

	UPROPERTY()
	TObjectPtr<UTacLogSubsystem> LogSubsystem;

	UPROPERTY()
	TObjectPtr<UTacticalPresentationBuilderConfig> Config;

	// Preload handles keep manifests and their visual content resident between BeginPreload and Build.
	TSharedPtr<FStreamableHandle> ManifestHandle;
	TSharedPtr<FStreamableHandle> ContentHandle;
};
