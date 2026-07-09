#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/Ticker.h"
#include "FloatingTextActor.generated.h"

class UTextRenderComponent;

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
	TObjectPtr<UTextRenderComponent> TextRender;

private:
	bool OnTick(float DeltaTime);

	FTSTicker::FDelegateHandle TickerHandle;
	FVector StartLocation = FVector::ZeroVector;
	FLinearColor StartColor = FLinearColor::White;
	float Elapsed = 0.f;

	static constexpr float RiseDistance = 100.f;
	static constexpr float Duration = 1.2f;
};
