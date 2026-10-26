#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FloatingTextWidget.generated.h"

class UTextBlock;

// UMG backing for AFloatingTextActor; assign a Blueprint subclass with the TextBlock bound and outline styled.
UCLASS()
class KBS_API UFloatingTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupText(const FText& Text, const FLinearColor& Color);
	void SetFadeColor(const FLinearColor& Color);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock;
};
