#include "UI/Tactical/Controller/TacticalGameController.h"
#include "UI/Tactical/HUD/TacticalHUD.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacSubsystemControl.h"

void ATacticalGameController::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		UTacSubsystemControl* Control = World->GetSubsystem<UTacSubsystemControl>();
		if (Control->IsReadyForStart())
		{
			OnSubsystemInitComplete();
		}
		else
		{
			Control->ReadyForStart.AddDynamic(this, &ATacticalGameController::OnSubsystemInitComplete);
		}
	}
}

void ATacticalGameController::OnSubsystemInitComplete()
{
	checkf(TacticalHUDClass, TEXT("TacticalHUDClass is not set in controller Blueprint"));
	TacticalHUD = CreateWidget<UTacticalHUD>(this, TacticalHUDClass);
	if (TacticalHUD)
	{
		TacticalHUD->AddToViewport();
	}
}

