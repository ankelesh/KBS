#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleEffectSlotSnapshot.generated.h"

class UBattleEffect;
class UImage;
class UTextBlock;
class UBorder;

// Lightweight read-only display of a battle effect - no event binding
// Shows icon, duration, frame color, and tooltip only
UCLASS(Blueprintable)
class KBS_API UBattleEffectSlotSnapshot : public UUserWidget
{
	GENERATED_BODY()

public:
	// Setup from effect data (read-only, no event binding)
	UFUNCTION(BlueprintCallable, Category = "Battle Effect Slot Snapshot")
	void SetupFromEffect(UBattleEffect* Effect);

	// Clear and hide widget
	UFUNCTION(BlueprintCallable, Category = "Battle Effect Slot Snapshot")
	void Clear();

protected:
	// UI Components
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UBorder* EffectFrameBorder;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UImage* EffectIcon;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock* DurationText;

	// Frame colors by effect type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor PositiveFrameColor = FLinearColor(0.0f, 0.8f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor NegativeFrameColor = FLinearColor(0.8f, 0.0f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor NeutralFrameColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor ImmutableFrameColor = FLinearColor(0.8f, 0.6f, 0.0f, 1.0f);
};
