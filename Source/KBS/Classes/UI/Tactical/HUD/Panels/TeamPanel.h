#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "TeamPanel.generated.h"

class UTeamTable;
class UBattleTeam;
class AUnit;
class UTacGridSubsystem;
class UTacTurnSubsystem;
class UWidgetSwitcher;
class UButton;

UCLASS(Blueprintable)
class KBS_API UTeamPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Team Panel")
	void Clear();

	UFUNCTION(BlueprintCallable, Category = "Team Panel")
	void ShowPlayerTeam();

	UFUNCTION(BlueprintCallable, Category = "Team Panel")
	void ShowAITeam();

	UFUNCTION(BlueprintCallable, Category = "Team Panel")
	void ShowAttackerTeam();

	UFUNCTION(BlueprintCallable, Category = "Team Panel")
	void ShowDefenderTeam();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Team Panel|Events")
	void BP_OnTeamSwapped(ETeamSide NewSide);

	UFUNCTION(BlueprintImplementableEvent, Category = "Team Panel|Events")
	void BP_OnSwapButtonClicked();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> TeamSwitcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> SwapButton;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Classes")
	TSubclassOf<UTeamTable> TeamTableClass;

private:
	UFUNCTION()
	void OnSwapButtonClicked();

	UFUNCTION()
	void OnTurnStarted(AUnit* Unit);

	void ShowTeam(ETeamSide Side);
	UBattleTeam* GetAITeam() const;

	UPROPERTY()
	TObjectPtr<UTacGridSubsystem> GridSubsystem = nullptr;

	UPROPERTY()
	TObjectPtr<UTacTurnSubsystem> TurnSubsystem = nullptr;

	UPROPERTY()
	TObjectPtr<UBattleTeam> AttackerTeam = nullptr;

	UPROPERTY()
	TObjectPtr<UBattleTeam> DefenderTeam = nullptr;

	UPROPERTY()
	TObjectPtr<UTeamTable> AttackerTable = nullptr;

	UPROPERTY()
	TObjectPtr<UTeamTable> DefenderTable = nullptr;

	ETeamSide DisplayedTeamSide = ETeamSide::Attacker;
};
