#include "GameMechanics/Picker/PickerGameMode.h"
#include "UI/Picker/Controller/PickerPlayerController.h"

APickerGameMode::APickerGameMode()
{
	PlayerControllerClass = APickerPlayerController::StaticClass();
}
