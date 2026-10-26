#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/Ticker.h"
#include "FloatingTextActor.generated.h"

class UWidgetComponent;
class UFloatingTextWidget;

// Self-contained in-world text popup: rises and fades over a fixed duration, then destroys itself.
UCLASS()
class KBS_API AFloatingTextActor : public AActor
{
	GENERATED_BODY()

public:
	AFloatingTextActor();

	void Init(const FText& Text, const FLinearColor& Color);

protected:
	UPROPERTY(VisibleAnywhere, Category = "FloatingText")
	TObjectPtr<UWidgetComponent> WidgetComponent;

	// Assign a Blueprint subclass (TextBlock bound, outline styled) once one exists.
	UPROPERTY(EditDefaultsOnly, Category = "FloatingText")
	TSubclassOf<UFloatingTextWidget> WidgetClass;

private:
	bool OnTick(float DeltaTime);
	void UpdateFacing();

	// Slow ease for the first FastFadeStart of Duration, quick ease for the remainder.
	static float ComputeFadeAlpha(float t);

	FTSTicker::FDelegateHandle TickerHandle;
	FVector StartLocation = FVector::ZeroVector;
	FLinearColor StartColor = FLinearColor::White;
	float Elapsed = 0.f;

	static constexpr float RiseDistance = 100.f;
	static constexpr float Duration = 1.2f;
	static constexpr float FastFadeStart = 0.9f;
	static constexpr float FastFadeStartAlpha = 0.75f;
};
