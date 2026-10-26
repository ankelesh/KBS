#include "UI/Common/Widgets/FloatingTextWidget.h"
#include "Components/TextBlock.h"

void UFloatingTextWidget::SetupText(const FText& Text, const FLinearColor& Color)
{
	TextBlock->SetText(Text);
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
}

void UFloatingTextWidget::SetFadeColor(const FLinearColor& Color)
{
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
}
