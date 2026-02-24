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
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("HandleStartingNewPlayer called!"));
		ApplyModeRulesToPawn(NewPlayer->GetPawn());
	}
}

void AAmaruGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
	if (NewPlayer && NewPlayer->GetPawn())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("RestartPlayer called!"));
		ApplyModeRulesToPawn(NewPlayer->GetPawn());
	}
}

void AAmaruGameMode::ApplyModeRulesToPawn(APawn* Pawn)
{
	if (!Pawn) return;

	AAmaruShooterCharacter* Char = Cast<AAmaruShooterCharacter>(Pawn);
	if (!Char) return;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("ApplyModeRulesToPawn called on AmaruShooterCharacter!"));

	if (bAbilitiesEnabled)
	{
		Char->Server_EnableAbilitiesForMode();
	}
	else
	{
		Char->Server_DisableAbilitiesForMode();
	}
}
