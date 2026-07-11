// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/AmaruGM_Deathmatch.h"

#include "AmaruGameState.h"
#include "AmaruShooter/AmaruPlayerState.h"

AAmaruGM_Deathmatch::AAmaruGM_Deathmatch()
	: Super()
{
	bHeroAbilitiesEnabled = false;
	MatchDuration = 300.f;
}

void AAmaruGM_Deathmatch::AssignTeam(AAmaruPlayerState* PS)
{
	// FFA: cada jugador es su propio equipo, así el friendly fire nunca bloquea.
	if (PS)
	{
		PS->SetTeamId(NextFFATeamId++);
	}
}

FName AAmaruGM_Deathmatch::GetPlayerStartTagForPlayer(const AAmaruPlayerState* PS) const
{
	// FFA: cualquier PlayerStart sirve.
	return NAME_None;
}

void AAmaruGM_Deathmatch::OnPlayerKilled(AAmaruPlayerState* Killer, AAmaruPlayerState* Victim)
{
	Super::OnPlayerKilled(Killer, Victim);

	if (KillLimit > 0 && Killer && Killer->Kills >= KillLimit)
	{
		DecideWinnerAndEnd();
	}
}

void AAmaruGM_Deathmatch::OnMatchTimeExpired()
{
	DecideWinnerAndEnd();
}

void AAmaruGM_Deathmatch::DecideWinnerAndEnd()
{
	if (!IsMatchInProgress()) return;

	AAmaruGameState* GS = GetGameState<AAmaruGameState>();
	if (GS)
	{
		AAmaruPlayerState* Best = nullptr;
		for (APlayerState* PS : GS->PlayerArray)
		{
			AAmaruPlayerState* APS = Cast<AAmaruPlayerState>(PS);
			if (!APS) continue;
			if (!Best ||
				APS->Kills > Best->Kills ||
				(APS->Kills == Best->Kills && APS->Deaths < Best->Deaths))
			{
				Best = APS;
			}
		}
		GS->WinningPlayer = Best;
	}
	EndMatch();
}
