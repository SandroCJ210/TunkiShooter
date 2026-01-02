// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "AmaruGameMode.generated.h"

/**
 * 
 */
UCLASS()
class AMARUSHOOTER_API AAmaruGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AAmaruGameMode();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mode Rules")
	bool bAbilitiesEnabled = true;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	void ApplyModeRulesToPawn(APawn* Pawn);
};
