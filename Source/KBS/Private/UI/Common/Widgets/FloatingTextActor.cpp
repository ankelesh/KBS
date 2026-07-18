#include "UI/Common/Widgets/FloatingTextActor.h"
#include "UI/Common/Widgets/FloatingTextWidget.h"
#include "Components/WidgetComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const TCHAR* FloatingTextWidgetPath = TEXT("/Game/_KBS/Blueprints/HUD/Tactical/Labels/WBP_FloatingText.WBP_FloatingText_C");
	const FVector2D FloatingTextDrawSize(200.f, 50.f);
}

AFloatingTextActor::AFloatingTextActor()
{
	PrimaryActorTick.bCanEverTick = false;

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	RootComponent = WidgetComponent;
	WidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	WidgetComponent->SetDrawSize(FloatingTextDrawSize);
	WidgetComponent->SetBlendMode(EWidgetBlendMode::Transparent);

	static ConstructorHelpers::FClassFinder<UFloatingTextWidget> WidgetFinder(FloatingTextWidgetPath);
	checkf(WidgetFinder.Succeeded(), TEXT("AFloatingTextActor: missing %s"), FloatingTextWidgetPath);
	WidgetClass = WidgetFinder.Class;
	WidgetComponent->SetWidgetClass(WidgetClass);
}

void AFloatingTextActor::Init(const FText& Text, const FLinearColor& Color)
{
	UFloatingTextWidget* Widget = Cast<UFloatingTextWidget>(WidgetComponent->GetUserWidgetObject());
	checkf(Widget, TEXT("FloatingTextActor requires WidgetClass to be assigned to a UFloatingTextWidget Blueprint"));
	Widget->SetupText(Text, Color);

	StartLocation = GetActorLocation();
	StartColor = Color;
	Elapsed = 0.f;

	UpdateFacing();

	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &AFloatingTextActor::OnTick), 0.f);
}

bool AFloatingTextActor::OnTick(float DeltaTime)
{
	Elapsed += DeltaTime;
	const float t = FMath::Clamp(Elapsed / Duration, 0.f, 1.f);

	SetActorLocation(StartLocation + FVector(0.f, 0.f, RiseDistance * t));
	UpdateFacing();

	FLinearColor FadedColor = StartColor;
	FadedColor.A = ComputeFadeAlpha(t);
	Cast<UFloatingTextWidget>(WidgetComponent->GetUserWidgetObject())->SetFadeColor(FadedColor);

	if (t >= 1.f)
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
		Destroy();
		return false;
	}

	return true;
}

float AFloatingTextActor::ComputeFadeAlpha(float t)
{
	if (t < FastFadeStart)
	{
		const float SlowPhaseT = t / FastFadeStart;
		return FMath::Lerp(1.f, FastFadeStartAlpha, SlowPhaseT * SlowPhaseT * SlowPhaseT);
	}

	const float FastPhaseT = (t - FastFadeStart) / (1.f - FastFadeStart);
	return FMath::Lerp(FastFadeStartAlpha, 0.f, FastPhaseT * FastPhaseT);
}

void AFloatingTextActor::UpdateFacing()
{
	const APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}

	const FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	const FRotator LookAt = (CameraLocation - GetActorLocation()).Rotation();
	SetActorRotation(LookAt);
}
