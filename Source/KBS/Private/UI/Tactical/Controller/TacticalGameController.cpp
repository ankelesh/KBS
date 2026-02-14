#include "UI/Tactical/Controller/TacticalGameController.h"
#include "UI/Tactical/HUD/TacticalHUD.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "Blueprint/UserWidget.h"

void ATacticalGameController::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UTacGridSubsystem* GridSubsystem = World->GetSubsystem<UTacGridSubsystem>())
		{
			GridSubsystem->Ready.AddDynamic(this, &ATacticalGameController::OnGridSubsystemReady);
		}
	}
}

void ATacticalGameController::OnGridSubsystemReady(UTacGridSubsystem* Subsystem)
{
	if (TacticalHUDClass)
	{
		TacticalHUD = CreateWidget<UTacticalHUD>(this, TacticalHUDClass);
		if (TacticalHUD)
		{
			TacticalHUD->AddToViewport();
		}
	}
}
