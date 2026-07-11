#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "GameModes/AmaruGameMode.h"
#include "AmaruGM_Convoy.generated.h"

class ACapturePoint;
class AConvoyCart;

/**
 * Convoy 3v3 en dos rondas: capturar el punto desbloquea el carro; cada
 * checkpoint suma tiempo; al final se invierten los roles y gana el equipo
 * con más progreso (o el más rápido si ambos completan).
 */
UCLASS()
class AMARUSHOOTER_API AAmaruGM_Convoy : public AAmaruGameMode
{
	GENERATED_BODY()

public:
	AAmaruGM_Convoy();

	UPROPERTY(EditDefaultsOnly, Category = "Convoy")
	float RoundDuration = 240.f;

	UPROPERTY(EditDefaultsOnly, Category = "Convoy")
	float CaptureBonusTime = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "Convoy")
	float CheckpointBonusTime = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "Convoy")
	float RoundResetTime = 8.f;

protected:
	virtual void HandleMatchHasStarted() override;
	virtual void OnMatchTimeExpired() override;
	virtual FName GetPlayerStartTagForPlayer(const AAmaruPlayerState* PS) const override;

	void StartRound(int32 RoundNumber);
	void EndRound(bool bCompleted);
	void DecideWinner();
	void AddBonusTime(float Seconds);
	void RespawnAllPlayers();

	void HandlePointCaptured();
	void HandleCheckpointReached(int32 CheckpointIndex);
	void HandleCartReachedEnd();

	UPROPERTY(Transient)
	TObjectPtr<ACapturePoint> CapturePoint;

	UPROPERTY(Transient)
	TObjectPtr<AConvoyCart> ConvoyCart;

	float RoundStartServerTime = 0.f;
	FTimerHandle RoundResetTimerHandle;
};
