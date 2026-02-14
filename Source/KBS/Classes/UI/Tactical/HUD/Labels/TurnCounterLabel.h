#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TurnCounterLabel.generated.h"

class UTacTurnSubsystem;

UCLASS()
class KBS_API UTurnCounterLabel : public UUserWidget
{
	GENERATED_BODY()

public:
	// Refresh display with current turn number
	void Refresh();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// UI Components
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UBorder* TurnBorder;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock* TurnNumber;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock* Explanation;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTacTurnSubsystem> TurnSubsystem = nullptr;

private:
	UFUNCTION()
	void OnRoundStarted(int32 Turn);
	
public:
	// Blueprint hook for custom visual styling when turn updates
	UFUNCTION(BlueprintImplementableEvent, Category = "Turn Counter|Visuals")
	void BP_OnTurnUpdated(int32 Turn);
};
