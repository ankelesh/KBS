#include "UI/Tactical/HUD/Labels/TurnCounterLabel.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"

void UTurnCounterLabel::NativeConstruct()
{
	Super::NativeConstruct();

	// Create root HorizontalBox
	UHorizontalBox* Root = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	WidgetTree->RootWidget = Root;

	// Create TurnIcon
	TurnBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Root->AddChildToHorizontalBox(TurnBorder);

	// Create TurnNumber text
	TurnNumber = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TurnNumber->SetText(FText::FromString(TEXT("0")));
	Root->AddChildToHorizontalBox(TurnNumber);
	
	Explanation = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Explanation->SetText(FText::FromString(TEXT("Turn:")));
	Root->AddChildToHorizontalBox(Explanation);

	// Get TurnSubsystem and bind to OnRoundStart
	if (UWorld* World = GetWorld())
	{
		TurnSubsystem = World->GetSubsystem<UTacTurnSubsystem>();
		if (TurnSubsystem)
		{
			TurnSubsystem->OnRoundStart.AddDynamic(this, &UTurnCounterLabel::OnRoundStarted);
			Refresh(); // Initial display
		}
	}
}

void UTurnCounterLabel::NativeDestruct()
{
	// Unbind from TurnSubsystem
	if (TurnSubsystem)
	{
		TurnSubsystem->OnRoundStart.RemoveDynamic(this, &UTurnCounterLabel::OnRoundStarted);
		TurnSubsystem = nullptr;
	}

	Super::NativeDestruct();
}

void UTurnCounterLabel::OnRoundStarted(int32 Turn)
{
	Refresh();
}

void UTurnCounterLabel::Refresh()
{
	if (!TurnSubsystem || !TurnNumber) return;

	int32 CurrentRound = TurnSubsystem->GetCurrentRound();
	FString TurnText = FString::Printf(TEXT("Turn: %d"), CurrentRound);
	TurnNumber->SetText(FText::FromString(TurnText));

	// Call BP hook for custom styling
	BP_OnTurnUpdated(CurrentRound);
}
