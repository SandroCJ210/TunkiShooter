// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "GameFramework/GameMode.h"
#include "AmaruGameMode.generated.h"

class AAmaruPlayerState;
class UGameplayEffect;

/**
 *
 */
UCLASS()
class AMARUSHOOTER_API AAmaruGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AAmaruGameMode();

	// false = solo arma primaria + recarga (Deathmatch); true = kit completo.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mode Rules")
	bool bHeroAbilitiesEnabled = true;

	UPROPERTY(EditDefaultsOnly, Category = "Mode Rules")
	float MatchDuration = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Mode Rules")
	float RespawnDelay = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Mode Rules")
	float AssistWindow = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Mode Rules")
	int32 NumTeams = 2;

	// Cura aplicada al asesino si tiene State.Rage (Furia del Yawar).
	UPROPERTY(EditDefaultsOnly, Category = "Mode Rules")
	TSubclassOf<UGameplayEffect> RageHealOnKillEffect;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void GenericPlayerInitialization(AController* C) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void HandleMatchHasStarted() override;

	void ApplyModeRulesToPawn(APawn* Pawn);

	// Llamado por el AttributeSet (servidor) cuando la vida llega a 0.
	void NotifyPlayerDied(AAmaruPlayerState* VictimPS, AActor* ContextInstigator);

protected:
	virtual void AssignTeam(AAmaruPlayerState* PS);
	virtual void OnMatchTimeExpired();
	virtual void OnPlayerKilled(AAmaruPlayerState* Killer, AAmaruPlayerState* Victim);
	virtual FName GetPlayerStartTagForPlayer(const AAmaruPlayerState* PS) const;

	void ScheduleRespawn(AController* Controller);
	void CancelPendingRespawns();

	FTimerHandle MatchTimerHandle;
	TMap<TWeakObjectPtr<AController>, FTimerHandle> PendingRespawns;
};
