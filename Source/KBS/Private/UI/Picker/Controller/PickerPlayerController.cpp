#include "UI/Picker/Controller/PickerPlayerController.h"
#include "UI/Picker/HUD/PickerHUD.h"

void APickerPlayerController::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());

	if (PickerHUDClass)
	{
		PickerHUD = CreateWidget<UPickerHUD>(this, PickerHUDClass);
		PickerHUD->AddToViewport();
	}
}
