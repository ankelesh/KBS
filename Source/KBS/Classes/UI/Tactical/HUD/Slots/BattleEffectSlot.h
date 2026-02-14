#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleEffectSlot.generated.h"

class UBattleEffect;
enum class EEffectType : uint8;

// Display widget for a single battle effect showing icon, duration, and frame
// Max 5 slots per unit, listens to OnDurationChange delegate
UCLASS(Blueprintable)
class KBS_API UBattleEffectSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	// Setup the slot with a battle effect (BP invokable)
	UFUNCTION(BlueprintCallable, Category = "Battle Effect Slot")
	void SetupEffect(UBattleEffect* Effect);

	// Clear the slot and unbind from effect
	UFUNCTION(BlueprintCallable, Category = "Battle Effect Slot")
	void Clear();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// UI Components - bind these in the UMG designer
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class USizeBox* SizeBox;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UOverlay* SlotRoot;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UBorder* EffectFrameBorder;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UImage* EffectIcon;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock* DurationText;

	// Frame colors by effect type (configure in BP or defaults)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor PositiveFrameColor = FLinearColor(0.0f, 0.8f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor NegativeFrameColor = FLinearColor(0.8f, 0.0f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor NeutralFrameColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor ImmutableFrameColor = FLinearColor(0.8f, 0.6f, 0.0f, 1.0f);

	// Currently bound effect
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UBattleEffect> BoundEffect = nullptr;

private:
	UFUNCTION()
	void OnEffectDurationChanged(int32 NewDuration);

	void UpdateDurationDisplay();
	void UpdateEffectFrame();
	void UpdateEffectIcon();
	void UpdateTooltip();
};
