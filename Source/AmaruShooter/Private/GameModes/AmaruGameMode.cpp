// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/AmaruGameMode.h"

#include "AmaruShooter/AmaruShooterCharacter.h"

AAmaruGameMode::AAmaruGameMode()
	: Super()
{

}

void AAmaruGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	if (NewPlayer && NewPlayer->GetPawn())
	{
		ApplyModeRulesToPawn(NewPlayer->GetPawn());
	}
}

void AAmaruGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
	if (NewPlayer && NewPlayer->GetPawn())
	{
		ApplyModeRulesToPawn(NewPlayer->GetPawn());
	}
}

void AAmaruGameMode::ApplyModeRulesToPawn(APawn* Pawn)
{
	if (!Pawn) return;

	AAmaruShooterCharacter* Char = Cast<AAmaruShooterCharacter>(Pawn);
	if (!Char) return;

	if (bAbilitiesEnabled)
	{
		Char->Server_EnableAbilitiesForMode();
	}
	else
	{
		Char->Server_DisableAbilitiesForMode();
	}
}
