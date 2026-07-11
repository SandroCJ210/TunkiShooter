#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "AmaruGameState.generated.h"

UENUM(BlueprintType)
enum class EConvoyPhase : uint8
{
	None,
	Capture,
	Escort,
	RoundReset,
	Finished
};

USTRUCT(BlueprintType)
struct FConvoyRoundResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bCaptured = false;

	UPROPERTY(BlueprintReadOnly)
	bool bCompleted = false;

	UPROPERTY(BlueprintReadOnly)
	int32 CheckpointsReached = 0;

	UPROPERTY(BlueprintReadOnly)
	float DistanceAlongSpline = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float TimeUsed = 0.f;
};

UCLASS()
class AMARUSHOOTER_API AAmaruGameState : public AGameState
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
	TArray<int32> TeamScores;

	// Momento (tiempo de servidor) en que el reloj llega a 0. El bonus por
	// checkpoint solo empuja este valor; los clientes derivan el restante.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
	float MatchEndServerTime = 0.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
	int32 WinningTeam = INDEX_NONE;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
	TObjectPtr<APlayerState> WinningPlayer;

	UFUNCTION(BlueprintPure, Category = "Match")
	float GetMatchTimeRemaining() const
	{
		return FMath::Max(0.f, MatchEndServerTime - GetServerWorldTimeSeconds());
	}

	// ---- Convoy ----

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Convoy")
	EConvoyPhase ConvoyPhase = EConvoyPhase::None;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Convoy")
	int32 CurrentRound = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Convoy")
	int32 AttackingTeamId = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Convoy")
	TArray<FConvoyRoundResult> RoundResults;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
