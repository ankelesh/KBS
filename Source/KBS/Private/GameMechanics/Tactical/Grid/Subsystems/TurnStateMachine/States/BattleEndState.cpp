#include "GameMechanics/Tactical/Grid/Subsystems/TurnStateMachine/States/BattleEndState.h"
#include "GameMechanics/Tactical/Grid/Subsystems/TacTurnSubsystem.h"
#include "KbsGameInstance.h"
#include "Kismet/GameplayStatics.h"

void FBattleEndState::Enter()
{
	UE_LOG(LogKBSTurn, Log, TEXT("Battle ended"));
	BroadcastBattleEnd();

	UWorld* World = ParentTurnSubsystem->GetWorld();
	UKbsGameInstance* GI = World->GetGameInstance<UKbsGameInstance>();
	if (GI && GI->HasPendingBattle())
	{
		const FName ReturnLevel = GI->GetPendingBattle().ReturnLevelName;
		GI->ClearPendingBattle();
		if (!ReturnLevel.IsNone())
		{
			UGameplayStatics::OpenLevel(World, ReturnLevel);
		}
	}
}

void FBattleEndState::Exit()
{
	UE_LOG(LogTemp, Warning, TEXT("Someone tries to tranzit out FBattleEndState"));
}

ETurnState FBattleEndState::NextState()
{
	return ETurnState::EBattleEndState;
}
