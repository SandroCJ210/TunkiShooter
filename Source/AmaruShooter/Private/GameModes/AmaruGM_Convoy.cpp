#include "GameModes/AmaruGM_Convoy.h"

#include "AmaruGameState.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "Objectives/CapturePoint.h"
#include "Objectives/ConvoyCart.h"
#include "EngineUtils.h"
#include "TimerManager.h"

AAmaruGM_Convoy::AAmaruGM_Convoy()
	: Super()
{
	bHeroAbilitiesEnabled = true;
}

void AAmaruGM_Convoy::HandleMatchHasStarted()
{
	// No llamar al Super de AAmaruGameMode a nivel de reloj único: el Convoy
	// gestiona su propio reloj por ronda. AGameMode::HandleMatchHasStarted sí.
	AGameMode::HandleMatchHasStarted();

	if (AAmaruGameState* GS = GetGameState<AAmaruGameState>())
	{
		GS->TeamScores.SetNumZeroed(FMath::Max(NumTeams, 1));
		GS->RoundResults.SetNum(2);
	}

	for (TActorIterator<ACapturePoint> It(GetWorld()); It; ++It)
	{
		CapturePoint = *It;
		break;
	}
	for (TActorIterator<AConvoyCart> It(GetWorld()); It; ++It)
	{
		ConvoyCart = *It;
		break;
	}

	if (!CapturePoint || !ConvoyCart)
	{
		UE_LOG(LogTemp, Error, TEXT("AmaruGM_Convoy: falta ACapturePoint o AConvoyCart en el mapa."));
		return;
	}

	CapturePoint->OnCapturedNative.AddUObject(this, &AAmaruGM_Convoy::HandlePointCaptured);
	ConvoyCart->OnCheckpointReachedNative.AddUObject(this, &AAmaruGM_Convoy::HandleCheckpointReached);
	ConvoyCart->OnReachedEndNative.AddUObject(this, &AAmaruGM_Convoy::HandleCartReachedEnd);

	StartRound(1);
}

FName AAmaruGM_Convoy::GetPlayerStartTagForPlayer(const AAmaruPlayerState* PS) const
{
	if (!PS || PS->GetTeamId() == INDEX_NONE) return NAME_None;

	const AAmaruGameState* GS = GetGameState<AAmaruGameState>();
	const int32 AttackingTeam = GS ? GS->AttackingTeamId : 0;
	return PS->GetTeamId() == AttackingTeam ? FName("Attack") : FName("Defense");
}

void AAmaruGM_Convoy::StartRound(int32 RoundNumber)
{
	AAmaruGameState* GS = GetGameState<AAmaruGameState>();
	if (!GS || !CapturePoint || !ConvoyCart) return;

	CancelPendingRespawns();

	GS->CurrentRound = RoundNumber;
	GS->AttackingTeamId = RoundNumber == 1 ? 0 : 1;
	GS->ConvoyPhase = EConvoyPhase::Capture;

	RoundStartServerTime = GS->GetServerWorldTimeSeconds();
	GS->MatchEndServerTime = RoundStartServerTime + RoundDuration;
	GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &AAmaruGM_Convoy::OnMatchTimeExpired, RoundDuration);

	CapturePoint->ResetPoint();
	ConvoyCart->NetMulticast_ResetCart();
	CapturePoint->SetPointActive(GS->AttackingTeamId);

	RespawnAllPlayers();
}

void AAmaruGM_Convoy::RespawnAllPlayers()
{
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AController* Controller = It->Get();
		if (Controller && Controller->GetPlayerState<AAmaruPlayerState>())
		{
			if (APawn* Pawn = Controller->GetPawn())
			{
				Pawn->Destroy();
			}
			RestartPlayer(Controller);
		}
	}
}

void AAmaruGM_Convoy::AddBonusTime(float Seconds)
{
	AAmaruGameState* GS = GetGameState<AAmaruGameState>();
	if (!GS) return;

	GS->MatchEndServerTime += Seconds;
	const float Remaining = GS->GetMatchTimeRemaining();
	GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &AAmaruGM_Convoy::OnMatchTimeExpired, FMath::Max(Remaining, 0.1f));
}

void AAmaruGM_Convoy::HandlePointCaptured()
{
	AAmaruGameState* GS = GetGameState<AAmaruGameState>();
	if (!GS || GS->ConvoyPhase != EConvoyPhase::Capture) return;

	GS->ConvoyPhase = EConvoyPhase::Escort;
	if (GS->RoundResults.IsValidIndex(GS->CurrentRound - 1))
	{
		GS->RoundResults[GS->CurrentRound - 1].bCaptured = true;
	}
	AddBonusTime(CaptureBonusTime);
	ConvoyCart->UnlockCart(GS->AttackingTeamId);
}

void AAmaruGM_Convoy::HandleCheckpointReached(int32 CheckpointIndex)
{
	AAmaruGameState* GS = GetGameState<AAmaruGameState>();
	if (!GS || GS->ConvoyPhase != EConvoyPhase::Escort) return;

	if (GS->RoundResults.IsValidIndex(GS->CurrentRound - 1))
	{
		GS->RoundResults[GS->CurrentRound - 1].CheckpointsReached = CheckpointIndex;
	}
	AddBonusTime(CheckpointBonusTime);
}

void AAmaruGM_Convoy::HandleCartReachedEnd()
{
	EndRound(true);
}

void AAmaruGM_Convoy::OnMatchTimeExpired()
{
	EndRound(false);
}

void AAmaruGM_Convoy::EndRound(bool bCompleted)
{
	AAmaruGameState* GS = GetGameState<AAmaruGameState>();
	if (!GS || GS->ConvoyPhase == EConvoyPhase::RoundReset || GS->ConvoyPhase == EConvoyPhase::Finished) return;

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	CapturePoint->SetPointInactive();

	if (GS->RoundResults.IsValidIndex(GS->CurrentRound - 1))
	{
		FConvoyRoundResult& Result = GS->RoundResults[GS->CurrentRound - 1];
		Result.bCompleted = bCompleted;
		Result.CheckpointsReached = ConvoyCart->CheckpointsReached;
		Result.DistanceAlongSpline = ConvoyCart->DistanceAlongSpline;
		Result.TimeUsed = GS->GetServerWorldTimeSeconds() - RoundStartServerTime;
	}

	if (GS->CurrentRound == 1)
	{
		GS->ConvoyPhase = EConvoyPhase::RoundReset;
		GetWorldTimerManager().SetTimer(RoundResetTimerHandle, [this]()
		{
			StartRound(2);
		}, RoundResetTime, false);
	}
	else
	{
		DecideWinner();
	}
}

void AAmaruGM_Convoy::DecideWinner()
{
	AAmaruGameState* GS = GetGameState<AAmaruGameState>();
	if (!GS) return;

	GS->ConvoyPhase = EConvoyPhase::Finished;

	const FConvoyRoundResult& R1 = GS->RoundResults[0]; // atacó equipo 0
	const FConvoyRoundResult& R2 = GS->RoundResults[1]; // atacó equipo 1

	int32 Winner = INDEX_NONE;
	if (R1.bCompleted && R2.bCompleted)
	{
		if (!FMath::IsNearlyEqual(R1.TimeUsed, R2.TimeUsed))
		{
			Winner = R1.TimeUsed < R2.TimeUsed ? 0 : 1;
		}
	}
	else if (R1.bCompleted != R2.bCompleted)
	{
		Winner = R1.bCompleted ? 0 : 1;
	}
	else if (R1.bCaptured != R2.bCaptured)
	{
		Winner = R1.bCaptured ? 0 : 1;
	}
	else if (R1.CheckpointsReached != R2.CheckpointsReached)
	{
		Winner = R1.CheckpointsReached > R2.CheckpointsReached ? 0 : 1;
	}
	else if (!FMath::IsNearlyEqual(R1.DistanceAlongSpline, R2.DistanceAlongSpline))
	{
		Winner = R1.DistanceAlongSpline > R2.DistanceAlongSpline ? 0 : 1;
	}

	GS->WinningTeam = Winner;
	EndMatch();
}
