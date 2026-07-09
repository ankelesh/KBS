#include "UI/Common/Widgets/FloatingTextActor.h"
#include "Components/TextRenderComponent.h"

AFloatingTextActor::AFloatingTextActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRender"));
	RootComponent = TextRender;
	TextRender->SetHorizontalAlignment(EHTA_Center);
	TextRender->SetWorldSize(48.f);
}

void AFloatingTextActor::Init(const FText& Text, const FLinearColor& Color)
{
	TextRender->SetText(Text);
	TextRender->SetTextRenderColor(Color.ToFColor(false));

	StartLocation = GetActorLocation();
	StartColor = Color;
	Elapsed = 0.f;

	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &AFloatingTextActor::OnTick), 0.f);
}

bool AFloatingTextActor::OnTick(float DeltaTime)
{
	Elapsed += DeltaTime;
	const float t = FMath::Clamp(Elapsed / Duration, 0.f, 1.f);

	SetActorLocation(StartLocation + FVector(0.f, 0.f, RiseDistance * t));

	FLinearColor FadedColor = StartColor;
	FadedColor.A = 1.f - t;
	TextRender->SetTextRenderColor(FadedColor.ToFColor(false));

	if (t >= 1.f)
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
		Destroy();
		return false;
	}

	return true;
}
