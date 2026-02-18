#include "UI/Tactical/HUD/Panels/TeamPanel.h"
#include "UI/Tactical/HUD/Tables/TeamTable.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacGridSubsystem.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "GameMechanics/Tactical/Grid/BattleTeam.h"
#include "GameMechanics/Units/Unit.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"

void UTeamPanel::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogTemp, Error, TEXT("TeamPanel NativeConstruct: Class=%s, Outer=%s"),
		*GetClass()->GetName(), *GetOuter()->GetName());
	checkf(TeamTableClass, TEXT("TeamTableClass not set in TeamPanel"));

	GridSubsystem = GetWorld()->GetSubsystem<UTacGridSubsystem>();
	checkf(GridSubsystem, TEXT("TacGridSubsystem not found"));

	TurnSubsystem = GetWorld()->GetSubsystem<UTacTurnSubsystem>();
	checkf(TurnSubsystem, TEXT("TacTurnSubsystem not found"));

	AttackerTeam = GridSubsystem->GetAttackerTeam();
	DefenderTeam = GridSubsystem->GetDefenderTeam();

	AttackerTable = CreateWidget<UTeamTable>(this, TeamTableClass);
	TeamSwitcher->AddChild(AttackerTable);
	AttackerTable->SetTeam(AttackerTeam);

	DefenderTable = CreateWidget<UTeamTable>(this, TeamTableClass);
	TeamSwitcher->AddChild(DefenderTable);
	DefenderTable->SetTeam(DefenderTeam);

	SwapButton->OnClicked.AddDynamic(this, &UTeamPanel::OnSwapButtonClicked);
	TurnSubsystem->OnTurnStart.AddDynamic(this, &UTeamPanel::OnTurnStarted);

	if (AUnit* CurrentUnit = TurnSubsystem->GetCurrentUnit())
	{
		ShowTeam(CurrentUnit->GetTeamSide());
	}
	else
	{
		ShowTeam(GridSubsystem->GetPlayerTeam()->GetTeamSide());
	}
}

void UTeamPanel::NativeDestruct()
{
	TurnSubsystem->OnTurnStart.RemoveDynamic(this, &UTeamPanel::OnTurnStarted);
	SwapButton->OnClicked.RemoveDynamic(this, &UTeamPanel::OnSwapButtonClicked);

	Super::NativeDestruct();
}

void UTeamPanel::Clear()
{
	AttackerTable->Clear();
	DefenderTable->Clear();
}

void UTeamPanel::ShowPlayerTeam()
{
	ShowTeam(GridSubsystem->GetPlayerTeam()->GetTeamSide());
}

void UTeamPanel::ShowAITeam()
{
	ShowTeam(GetAITeam()->GetTeamSide());
}

void UTeamPanel::ShowAttackerTeam()
{
	ShowTeam(ETeamSide::Attacker);
}

void UTeamPanel::ShowDefenderTeam()
{
	ShowTeam(ETeamSide::Defender);
}

void UTeamPanel::ShowTeam(ETeamSide Side)
{
	DisplayedTeamSide = Side;
	TeamSwitcher->SetActiveWidgetIndex(static_cast<int32>(Side));
	BP_OnTeamSwapped(Side);
}

void UTeamPanel::OnSwapButtonClicked()
{
	BP_OnSwapButtonClicked();

	ETeamSide NewSide = (DisplayedTeamSide == ETeamSide::Attacker)
		? ETeamSide::Defender
		: ETeamSide::Attacker;

	ShowTeam(NewSide);
}

void UTeamPanel::OnTurnStarted(AUnit* Unit)
{
	ShowTeam(Unit->GetTeamSide());
}

UBattleTeam* UTeamPanel::GetAITeam() const
{
	ETeamSide PlayerSide = GridSubsystem->GetPlayerTeam()->GetTeamSide();
	return (PlayerSide == ETeamSide::Attacker) ? DefenderTeam.Get() : AttackerTeam.Get();
}
