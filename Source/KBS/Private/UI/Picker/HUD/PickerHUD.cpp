#include "UI/Picker/HUD/PickerHUD.h"
#include "UI/Picker/Panels/UnitRosterPanel.h"
#include "UI/Picker/Panels/TeamGridPanel.h"
#include "GameMechanics/Units/UnitDefinition.h"
#include "KbsGameInstance.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UPickerHUD::NativeConstruct()
{
	Super::NativeConstruct();

	for (int32 i = 0; i < NumTeams; ++i)
	{
		TeamSlots[i].Init(nullptr, TotalSlots);
	}

	RosterPanel->OnUnitInfoRequested.BindUObject(this, &UPickerHUD::OnUnitInfoRequested);
	TeamGridPanel->OnUnitDropped.BindUObject(this, &UPickerHUD::OnUnitDropped);
	TeamGridPanel->OnSlotCleared.BindUObject(this, &UPickerHUD::OnSlotCleared);

	UpdateActiveTeamLabel();
}

void UPickerHUD::SwapActiveTeam()
{
	ActiveTeamIndex = (ActiveTeamIndex + 1) % NumTeams;
	RefreshTeamGrid();
	UpdateActiveTeamLabel();
}

void UPickerHUD::SetAIControlledTeam(ETeamSide Team)
{
	AIControlledTeam = Team;
}

void UPickerHUD::TryStartBattle()
{
	TArray<FBattleUnitPlacement> Attacker = BuildTeamPlacements(0);
	TArray<FBattleUnitPlacement> Defender = BuildTeamPlacements(1);
	if (Attacker.IsEmpty() && Defender.IsEmpty()) return;

	FTacticalBattleSetup Setup;
	Setup.AttackerUnits = MoveTemp(Attacker);
	Setup.DefenderUnits = MoveTemp(Defender);
	Setup.AIControlledTeam = AIControlledTeam;
	Setup.ReturnLevelName = PickerLevelName;

	UKbsGameInstance* GI = GetWorld()->GetGameInstance<UKbsGameInstance>();
	checkf(GI, TEXT("PickerHUD: GameInstance is not UKbsGameInstance - set it in Project Settings"));
	GI->SetPendingBattle(Setup);

	UGameplayStatics::OpenLevel(GetWorld(), TacticalLevelName);
}

void UPickerHUD::OnUnitDropped(UUnitDefinition* Definition, int32 Row, int32 Col)
{
	if (!Definition || !CanPlaceUnit(Definition, Row, Col)) return;
	PlaceUnit(Definition, Row, Col);
	RefreshTeamGrid();
}

void UPickerHUD::OnSlotCleared(int32 Row, int32 Col)
{
	TArray<TObjectPtr<UUnitDefinition>>& Slots = GetActiveSlots();
	const int32 I0 = SlotIndex(Row, 0);
	const int32 I1 = SlotIndex(Row, 1);

	// If a 2-cell unit occupies this row, clear both
	if (Slots[I0] && Slots[I0] == Slots[I1])
	{
		Slots[I0] = nullptr;
		Slots[I1] = nullptr;
	}
	else
	{
		Slots[SlotIndex(Row, Col)] = nullptr;
	}
	RefreshTeamGrid();
}

void UPickerHUD::OnUnitInfoRequested(UUnitDefinition* Definition)
{
	// Info panel display is handled entirely in Blueprint (bind to this event from WBP)
}

bool UPickerHUD::CanPlaceUnit(UUnitDefinition* Definition, int32 Row, int32 Col) const
{
	const TArray<TObjectPtr<UUnitDefinition>>& Slots = GetSlots(ActiveTeamIndex);
	if (Definition->UnitSize == 2)
	{
		// 2-cell unit must occupy the full row; row must be entirely empty
		return !Slots[SlotIndex(Row, 0)] && !Slots[SlotIndex(Row, 1)];
	}
	// 1-cell unit: target cell must be empty and not part of a 2-cell unit's row
	const int32 I0 = SlotIndex(Row, 0);
	const int32 I1 = SlotIndex(Row, 1);
	const bool bRowHas2Cell = Slots[I0] && Slots[I0] == Slots[I1];
	return !bRowHas2Cell && !Slots[SlotIndex(Row, Col)];
}

void UPickerHUD::PlaceUnit(UUnitDefinition* Definition, int32 Row, int32 Col)
{
	TArray<TObjectPtr<UUnitDefinition>>& Slots = GetActiveSlots();
	if (Definition->UnitSize == 2)
	{
		Slots[SlotIndex(Row, 0)] = Definition;
		Slots[SlotIndex(Row, 1)] = Definition; // same pointer marks the 2-cell pair
	}
	else
	{
		Slots[SlotIndex(Row, Col)] = Definition;
	}
}

TArray<FBattleUnitPlacement> UPickerHUD::BuildTeamPlacements(int32 TeamIndex) const
{
	const TArray<TObjectPtr<UUnitDefinition>>& Slots = GetSlots(TeamIndex);
	TArray<FBattleUnitPlacement> Result;

	for (int32 Row = 0; Row < SlotRows; ++Row)
	{
		UUnitDefinition* S0 = Slots[SlotIndex(Row, 0)];
		UUnitDefinition* S1 = Slots[SlotIndex(Row, 1)];

		if (S0 && S0 == S1)
		{
			// 2-cell unit: emit one entry at col 0; DataManager handles the extra cell
			Result.Add({S0, Row, 0, ETacGridLayer::Ground});
		}
		else
		{
			if (S0) Result.Add({S0, Row, 0, ETacGridLayer::Ground});
			if (S1) Result.Add({S1, Row, 1, ETacGridLayer::Ground});
		}
	}
	return Result;
}

void UPickerHUD::RefreshTeamGrid()
{
	TeamGridPanel->RefreshFromSlots(GetActiveSlots());
}

void UPickerHUD::UpdateActiveTeamLabel()
{
	if (ActiveTeamLabel)
	{
		const FString TeamName = ActiveTeamIndex == 0 ? TEXT("Attacker") : TEXT("Defender");
		ActiveTeamLabel->SetText(FText::FromString(TeamName));
	}
}
