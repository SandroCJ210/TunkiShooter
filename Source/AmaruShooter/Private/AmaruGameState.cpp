#include "AmaruGameState.h"
#include "Net/UnrealNetwork.h"

void AAmaruGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAmaruGameState, TeamScores);
	DOREPLIFETIME(AAmaruGameState, MatchEndServerTime);
	DOREPLIFETIME(AAmaruGameState, WinningTeam);
	DOREPLIFETIME(AAmaruGameState, WinningPlayer);
	DOREPLIFETIME(AAmaruGameState, ConvoyPhase);
	DOREPLIFETIME(AAmaruGameState, CurrentRound);
	DOREPLIFETIME(AAmaruGameState, AttackingTeamId);
	DOREPLIFETIME(AAmaruGameState, RoundResults);
}
