#pragma once
#include "CoreMinimal.h"
#include "UI/Common/HUD/KbsHUD.h"
#include "GameplayTypes/TeamConstants.h"
#include "GameplayTypes/TacticalBattleSetup.h"
#include "PickerHUD.generated.h"

class UUnitDefinition;
class UUnitRosterPanel;
class UTeamGridPanel;
class UTextBlock;

UCLASS(Blueprintable)
class KBS_API UPickerHUD : public UKbsHUD
{
	GENERATED_BODY()
public:
	// Wired to Swap button in BP
	UFUNCTION(BlueprintCallable) void SwapActiveTeam();
	// Wired to Start Battle button in BP
	UFUNCTION(BlueprintCallable) void TryStartBattle();
	// Wired to AI-side checkbox in BP
	UFUNCTION(BlueprintCallable) void SetAIControlledTeam(ETeamSide Team);

	// Called by TeamGridPanel delegates
	void OnUnitDropped(UUnitDefinition* Definition, int32 Row, int32 Col);
	void OnSlotCleared(int32 Row, int32 Col);
	// Called by UnitRosterPanel delegate
	void OnUnitInfoRequested(UUnitDefinition* Definition);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<UUnitRosterPanel> RosterPanel;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<UTeamGridPanel> TeamGridPanel;

	// Optional: label showing which team is active ("Attacker" / "Defender")
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget)) TObjectPtr<UTextBlock> ActiveTeamLabel;

	// Name of the tactical level to open when starting battle
	UPROPERTY(EditDefaultsOnly, Category = "Picker") FName TacticalLevelName = FName("BaseTestGround");
	// Name of this picker level, stored as ReturnLevelName in the battle setup
	UPROPERTY(EditDefaultsOnly, Category = "Picker") FName PickerLevelName = FName("UnitPickerScene");

private:
	static constexpr int32 NumTeams = 2;
	static constexpr int32 SlotRows = 3;
	static constexpr int32 SlotCols = 2;
	static constexpr int32 TotalSlots = SlotRows * SlotCols;

	// [TeamIndex 0=Attacker 1=Defender][Row*SlotCols+Col] -> Definition (nullptr=empty, same ptr for 2-cell)
	UPROPERTY() TArray<TObjectPtr<UUnitDefinition>> TeamSlots[NumTeams];

	int32 ActiveTeamIndex = 0;
	ETeamSide AIControlledTeam = ETeamSide::Defender;

	TArray<TObjectPtr<UUnitDefinition>>& GetActiveSlots() { return TeamSlots[ActiveTeamIndex]; }
	const TArray<TObjectPtr<UUnitDefinition>>& GetSlots(int32 TeamIndex) const { return TeamSlots[TeamIndex]; }

	int32 SlotIndex(int32 Row, int32 Col) const { return Row * SlotCols + Col; }
	bool CanPlaceUnit(UUnitDefinition* Definition, int32 Row, int32 Col) const;
	void PlaceUnit(UUnitDefinition* Definition, int32 Row, int32 Col);

	TArray<FBattleUnitPlacement> BuildTeamPlacements(int32 TeamIndex) const;
	void RefreshTeamGrid();
	void UpdateActiveTeamLabel();
};
